#ifndef COMMLOG_H
#define COMMLOG_H

#include <QMainWindow>
#include <QTextEdit>

namespace Ui {
class CommLog;
}

class CommLog : public QMainWindow
{
    Q_OBJECT

public:
    explicit CommLog(QWidget *parent = nullptr);
    ~CommLog();

    QTextEdit * Transcript;
    void PostIncoming(QString msg);
    void PostOutgoing(QString msg);
    void PostInfo(QString msg);
    void PostError(QString msg);

private slots:
    void on_action_Close_triggered();

    void on_actionC_lear_triggered();

private:
    Ui::CommLog *ui;
};

#endif // COMMLOG_H
