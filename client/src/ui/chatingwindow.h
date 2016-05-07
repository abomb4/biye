#ifndef CHATINGWINDOW_H
#define CHATINGWINDOW_H

#include <QMainWindow>

namespace Ui {
class ChatingWindow;
}

class ChatingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatingWindow(QWidget *parent = 0);
    ~ChatingWindow();
    void touserid(uint32_t userid) { this->_to_user_id = userid; }
    uint32_t touserid() const { return this->_to_user_id; }

    void appendMsg(const QString &username, const QString &msg, const QDateTime &time);

private:
    uint32_t _to_user_id;
    Ui::ChatingWindow *ui;

public slots:
    // must run as main thread
    void receiveMsg(uint32_t from_user_id, uint32_t to_user_id, const QString &msgbody);

private slots:
    void _do_add_picture();
    void _do_send_msg();
};

#endif // CHATINGWINDOW_H
