/*
Copyright (c) 2013 Raivis Strogonovs

http://morf.lv

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#include "smtp.h"

//#define DEBUG

Smtp::Smtp( const QString &user, const QString &pass, const QString &host, int port, int timeout )
{    
    socket = new QSslSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected() ) );
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(errorReceived(QAbstractSocket::SocketError)));   
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));


    this->user = user;
    this->pass = pass;

    this->host = host;
    this->port = port;
    this->timeout = timeout;


}

void Smtp::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body, QStringList files)
{
    message = "To: " + to + "\n";    
    message.append("From: " + from + "\n");
    message.append("Subject: " + subject + "\n");

    //Let's intitiate multipart MIME with cutting boundary "frontier"
    message.append("MIME-Version: 1.0\n");
    message.append("Content-Type: multipart/mixed; boundary=frontier\n\n");



    message.append( "--frontier\n" );
    //message.append( "Content-Type: text/html\n\n" );  //Uncomment this for HTML formating, coment the line below
    message.append( "Content-Type: text/plain\n\n" );
    message.append(body);
    message.append("\n\n");

    if(!files.isEmpty())
    {
#ifdef DEBUG
        qDebug() << "Files to be sent: " << files.size();
#endif
        foreach(QString filePath, files)
        {
            QFile file(filePath);
            if(file.exists())
            {
                if (!file.open(QIODevice::ReadOnly))
                {
                    qDebug() << filePath << " Couldn't open";
#ifdef DEBUG
                    qDebug("Couldn't open the file");
#endif
                        return ;
                }
                QByteArray bytes = file.readAll();
                message.append( "--frontier\n" );
                message.append( "Content-Type: application/octet-stream\nContent-Disposition: attachment; filename="+ QFileInfo(file.fileName()).fileName() +";\nContent-Transfer-Encoding: base64\n\n" );
                message.append(bytes.toBase64());
                message.append("\n");
            } else {
                qDebug() << filePath << " not exists";
            }
        }
    }
#ifdef DEBUG
    else
        qDebug() << "No attachments found";
#endif


    message.append( "--frontier--\n" );

    message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
    message.replace( QString::fromLatin1( "\r\n.\r\n" ),QString::fromLatin1( "\r\n..\r\n" ) );


    this->from = from;
    rcpt = to;
    state = Init;
    socket->connectToHostEncrypted(host, port); //"smtp.gmail.com" and 465 for gmail TLS
//    socket->connectToHost(host, port);
    if (!socket->waitForConnected(timeout)) {
         qDebug() << socket->errorString();
     }

    t = new QTextStream( socket );



}

Smtp::~Smtp()
{
    delete t;
    delete socket;
}
void Smtp::stateChanged(QAbstractSocket::SocketState socketState)
{
#ifdef DEBUG
    qDebug() <<"stateChanged " << socketState;
#else
    Q_UNUSED(socketState);
#endif
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError)
{
#ifdef DEBUG
    qDebug() << "error " <<socketError;
#else
    Q_UNUSED(socketError);
#endif
}

void Smtp::disconnected()
{
#ifdef DEBUG
    qDebug() <<"disconneted";
    qDebug() << "error "  << socket->errorString();
#endif
}

void Smtp::connected()
{    
#ifdef DEBUG
    qDebug() << "Connected ";
#endif
}

void Smtp::readyRead()
{
#ifdef DEBUG
     qDebug() <<"readyRead";
#endif
    // SMTP is line-oriented

    QString responseLine;
    do
    {
        responseLine = socket->readLine();
        response += responseLine;
    }
    while ( socket->canReadLine() && responseLine[3] != ' ' );

    responseLine.truncate( 3 );
#ifdef DEBUG
    qDebug() << "Server response code:" <<  responseLine;
    qDebug() << "Server response: " << response;
#endif
    if ( state == Init && responseLine == "220" )
    {
        // banner was okay, let's go on
#ifdef DEBUG
        qDebug() << "Init :: " << "EHLO localhost" << "\r\n";
#endif
        *t << "EHLO localhost" << "\r\n";
        t->flush();

        state = Tls;
    }
    //No need, because I'm using socket->startClienEncryption() which makes the SSL handshake for you
    else if (state == Tls && responseLine == "250")
    {
        // Trying AUTH
#ifdef DEBUG
        qDebug() << "STarting Tls";
#endif
        *t << "STARTTLS" << "\r\n";
        t->flush();
        state = Auth;
    }
    else if (state == HandShake && responseLine == "220")
    {
#ifdef DEBUG
        qDebug() << "Start Client Encryption";
#endif
        socket->startClientEncryption();
        if(!socket->waitForEncrypted(timeout))
        {
            qDebug() << socket->errorString();
            state = Close;
        }

        //Send EHLO once again but now encrypted
#ifdef DEBUG
        qDebug() << "HandShake :: " << "EHLO localhost" << "\r\n";
#endif
        *t << "EHLO localhost" << "\r\n";
        t->flush();
        state = Auth;
    }
    else if (state == Auth && responseLine == "530")
    {
        // Trying AUTH
#ifdef DEBUG
        qDebug() << "Auth :: " << "AUTH LOGIN" << "\r\n";
#endif
        *t << "AUTH LOGIN" << "\r\n";
        t->flush();
        state = User;
    }
    else if (state == User && responseLine == "334")
    {
        //Trying User        
#ifdef DEBUG
        qDebug() << "Username :: " << QByteArray().append(user).toBase64()  << "\r\n";
#endif
        //GMAIL is using XOAUTH2 protocol, which basically means that password and username has to be sent in base64 coding
        //https://developers.google.com/gmail/xoauth2_protocol
        *t << QByteArray().append(user).toBase64()  << "\r\n";
        t->flush();

        state = Pass;
    }
    else if (state == Pass && responseLine == "334")
    {
        //Trying pass
#ifdef DEBUG
        qDebug() << "Pass :: " << QByteArray().append(pass).toBase64() << "\r\n";
#endif
        *t << QByteArray().append(pass).toBase64() << "\r\n";
        t->flush();

        state = Mail;
    }
    else if ( state == Mail && responseLine == "235" )
    {
        // HELO response was okay (well, it has to be)

        //Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way -> <email@gmail.com>
#ifdef DEBUG
        qDebug() << "Mail :: " << "MAIL FROM:<" << from << ">\r\n";
#endif
        *t << "MAIL FROM:<" << from << ">\r\n";
        t->flush();
        state = Rcpt;
    }
    else if ( state == Rcpt && responseLine == "250" )
    {
        //Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way -> <email@gmail.com>
#ifdef DEBUG
        qDebug() << "Rcpt :: " << "RCPT TO:<" << rcpt << ">\r\n";
#endif
        *t << "RCPT TO:<" << rcpt << ">\r\n";
        t->flush();
        state = Data;
    }
    else if ( state == Data && responseLine == "250" )
    {
#ifdef DEBUG
        qDebug() << "Data :: " << "DATA\r\n";
#endif
        *t << "DATA\r\n";
        t->flush();
        state = Body;
    }
    else if ( state == Body && responseLine == "354" )
    {
#ifdef DEBUG
        qDebug() << "Body :: " << message << "\r\n.\r\n";
#endif
        *t << message << "\r\n.\r\n";
        t->flush();
        state = Quit;
    }
    else if ( state == Quit && responseLine == "250" )
    {
#ifdef DEBUG
        qDebug() << "Quit :: " << "QUIT\r\n";
#endif
        *t << "QUIT\r\n";
        t->flush();
        // here, we just close.
        state = Close;
        emit status("Message sent to "+rcpt);
    }
    else if ( state == Close )
    {
        deleteLater();
        return;
    }
    else
    {
        // something broke.
        state = Close;
        emit status("Failed send message to "+rcpt);
    }
    response = "";
}
