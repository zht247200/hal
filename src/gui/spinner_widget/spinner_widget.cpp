#include "gui/spinner_widget/spinner_widget.h"

#include <QPainter>

spinner_widget::spinner_widget(QWidget* parent) : QWidget(parent),
    m_renderer(new QSvgRenderer())
{
    connect(m_renderer, &QSvgRenderer::repaintNeeded, this, &spinner_widget::handle_repaint_needed);

    const QString string(":/images/spinner");
    m_renderer->load(string);
}

void spinner_widget::handle_repaint_needed()
{
    update();
}

void spinner_widget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    m_renderer->render(&painter, rect());
}
