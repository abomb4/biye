#include "chatingwindow.h"
#include "ui_chatingwindow.h"

#include <QFileDialog>
#include <QImageReader>
#include <QStandardPaths>
#include <QTextBlock>
#include <QBuffer>

#include "fxclient.h"

#include <QMessageBox>
#include <QDebug>

#define IMG_MAX_WIDTH 200
#define IMG_MAX_HEIGHT 200

ChatingWindow::ChatingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatingWindow)
{
    ui->setupUi(this);
    connect(ui->btn_add_pic, SIGNAL(clicked(bool)), this, SLOT(_do_add_picture()));
    connect(ui->btn_send, SIGNAL(clicked(bool)), this, SLOT(_do_send_msg()));
}

ChatingWindow::~ChatingWindow() {
    this->destoryd(this->_to_user_id);
    delete ui;
}

void ChatingWindow::closeEvent(QCloseEvent *event) {
    delete this;
}

void ChatingWindow::_do_add_picture() {
    QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                    QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                    tr("JPG PNG (*.jpg *jpeg *.png)\n"));
    QImage image = QImageReader (file).read();
    static int rnum = 0;
    QUrl Uri(QString("%1_%2").arg(file).arg(rnum++));

    QTextDocument * textDocument = ui->edt_input->document();
    textDocument->addResource( QTextDocument::ImageResource, Uri, QVariant ( image ) );
    QTextCursor cursor = ui->edt_input->textCursor();
    QTextImageFormat imageFormat;
    double divide = 1;
    if (image.width() > IMG_MAX_WIDTH) {
        divide = (double)image.width() / IMG_MAX_WIDTH;
    }
    if (image.height() > IMG_MAX_HEIGHT) {
        double d = (double)image.height() / IMG_MAX_HEIGHT;
        if (d > divide)
            divide = d;
    }
    imageFormat.setWidth( image.width() / divide );
    imageFormat.setHeight( image.height() / divide );
    imageFormat.setName( Uri.toString() );

    cursor.insertImage(imageFormat);
}


void ChatingWindow::setContactWidget(QWidget *contact) {
    contact->setParent(this);
    this->ui->c_contact->layout()->addWidget(contact);
}

QString toFxBody(const QString &txts, const QStringList &images) {
    // TODO 1 convert all '&' to '&&'
    // 2 all '\n' to &n
    // 3 picture to &pbase64%P
    static QString placeHolder = QString::fromUtf8("\uFFFC");

    QString txt = txts;
    txt.replace('&', "&&");
    txt.replace('\n', "&n");
    int index = 0;
    for (int i = 0; i < images.size(); i++) {
        index = txt.indexOf(placeHolder, index);
        if (index == -1) {
            qDebug() << "早い！";
            break;
        }
        // convert to png and base64
        QImage image = QImageReader(images.at(i)).read();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        image.save(&buffer, "PNG"); // writes the image in PNG format inside the buffer
        QString base64edpic = QString::fromLatin1(byteArray.toBase64().data());

        txt.replace(index, placeHolder.length(), QString("%1%2%3").arg("&p").arg(base64edpic).arg("&P"));
        index++;
    }
    return txt;
}

// slots
QString fromFxBody(const QString &txts) {
    static QString special = "&";
    QTextDocument doc;
    QString txt = txts;
    txt.replace('<', "&&lt;");
    txt.replace('>', "&&gt;");
    txt.insert(0, "<p>");
    int index = 0;
    while(true) {
        index = txt.indexOf(special, index);
        if (index == -1) {
            break;
        }
        index++;
        switch(txt.at(index).toLatin1()) {
        case '&': // '&'
            txt.replace(index - 1, 2, "&");break;
        case 'n': // '\n'
            txt.replace(index - 1, 2, "</p><p>");break;
        case 'p': // picture
            txt.replace(index - 1, 2, "<img src='data:image/png;base64,");break;
        case 'P': // picture
            txt.replace(index - 1, 2, "'>");break;
        }
    }
    txt.append("</p>");
    return txt;
}

void ChatingWindow::_do_send_msg() {
    QStringList images;
    QTextBlock b = ui->edt_input->document()->begin();
    while (b.isValid()) {
        for (QTextBlock::iterator i = b.begin(); !i.atEnd(); ++i) {
            QTextCharFormat format = i.fragment().charFormat();
            bool isImage = format.isImageFormat();
            if (isImage) {
                QString name = format.toImageFormat().name();
                QString path = name.leftRef(name.lastIndexOf('_')).toString();
                images.append(path);
                qDebug() << "name: " << name
                         << "path: " << path;
            }
        }
        b = b.next();
    }
    QString txt = toFxBody(ui->edt_input->toPlainText(), images);
    FxChat::FxClient::sendMsg(this->_to_user_id, txt);
    ui->edt_show->append(fromFxBody(txt));
}

// must run as main thread
void ChatingWindow::receiveMsg(uint32_t from_user_id, uint32_t to_user_id, const QString &msgbody) {

}

void ChatingWindow::appendMsg(const QString &username, const QString &msg, const QDateTime &time) {
    // todo append to show area
    this->ui->edt_show->append(fromFxBody(msg));
}
