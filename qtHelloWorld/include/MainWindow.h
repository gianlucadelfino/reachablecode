#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent_) : QMainWindow(parent_), _button(this)
    {
        this->resize(400, 300);
        _button.setText("Push Here");
        _button.setGeometry(QRect(QPoint(100, 100), QSize(200, 50)));

        connect(&_button, SIGNAL(released()), this, SLOT(handleButton()));
    }

private slots:
    void handleButton()
    {
        _button.setText("Pressed!");
    }


private:
    QPushButton _button;
};
