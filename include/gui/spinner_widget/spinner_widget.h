#ifndef SPINNER_WIDGET_H
#define SPINNER_WIDGET_H

#include <QSvgRenderer>
#include <QWidget>

class spinner_widget final : public QWidget
{
    Q_OBJECT

public:
    spinner_widget(QWidget* parent = nullptr);

private Q_SLOTS:
    void handle_repaint_needed();

private:
    void paintEvent(QPaintEvent* event) override;

    QSvgRenderer* m_renderer;
};

#endif // SPINNER_WIDGET_H
