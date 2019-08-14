#include "graph_widget/items/nets/arrow_separated_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal arrow_separated_net::s_wire_length;
qreal arrow_separated_net::s_arrow_offset;
qreal arrow_separated_net::s_arrow_length;
qreal arrow_separated_net::s_arrow_height;
qreal arrow_separated_net::s_arrow_left;
qreal arrow_separated_net::s_arrow_right;

qreal arrow_separated_net::s_input_offset;

qreal arrow_separated_net::s_input_width;
qreal arrow_separated_net::s_output_width;

QPainterPath arrow_separated_net::s_arrow;

QBrush arrow_separated_net::s_brush;

void arrow_separated_net::load_settings()
{
    s_wire_length = 26;
    s_arrow_offset = 3;
    s_arrow_length = 12;
    s_arrow_height = 6;
    s_arrow_left = 0;
    s_arrow_right = 3;

    s_input_offset = s_wire_length + s_arrow_offset + s_arrow_length;

    if (s_arrow_right > 0)
        s_input_offset += s_arrow_right;

    s_input_width = s_wire_length + s_arrow_offset + s_arrow_length;

    if (s_arrow_left > 0)
        s_input_width += s_arrow_left;

    if (s_arrow_right < 0)
        s_input_width -= s_arrow_right;

    s_output_width = s_wire_length + s_arrow_offset + s_arrow_length;

    if (s_arrow_right < 0)
        s_output_width -= s_arrow_right;

    if (s_arrow_left > 0)
        s_output_width += s_arrow_left;

    s_arrow.lineTo(QPointF(s_arrow_left, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left + s_arrow_length, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left + s_arrow_length + s_arrow_right, 0));
    s_arrow.lineTo(QPointF(s_arrow_left + s_arrow_length, -(s_arrow_height / 2)));
    s_arrow.lineTo(QPointF(s_arrow_left, -(s_arrow_height / 2)));
    s_arrow.closeSubpath();

    s_brush.setStyle(Qt::SolidPattern);
    s_pen.setColor(QColor(160, 160, 160)); // USE STYLESHEETS
}

arrow_separated_net::arrow_separated_net(const std::shared_ptr<const net> n) : separated_graphics_net(n)
{

}

void arrow_separated_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    if (s_lod < graph_widget_constants::global_net_min_lod)
        return;

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    color.setAlphaF(s_alpha);

    s_pen.setColor(color);
    s_brush.setColor(color);
    painter->setPen(s_pen);
    painter->setBrush(s_brush);

    if (m_draw_output)
    {
        painter->drawLine(QPointF(0, 0), QPointF(s_wire_length, 0));
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->save();
        painter->translate(QPointF(s_wire_length + s_arrow_offset, 0));
        painter->drawPath(s_arrow);
        painter->restore();
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    for (const QPointF& position : m_input_wires)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->save();
        painter->translate(QPointF(position.x() - s_input_offset, position.y()));
        painter->drawPath(s_arrow);
        painter->restore();
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

//#ifdef HAL_DEBUG_GUI_GRAPHICS
    bool original_value = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(original_value);
//#endif

    painter->setBrush(QBrush());
}

void arrow_separated_net::set_visuals(const graphics_net::visuals& v)
{
    setVisible(v.visible);
    m_color = v.color;
    m_line_style = v.style;
}

void arrow_separated_net::add_output()
{
    if (m_draw_output)
        return;

    m_draw_output = true;

    m_shape.moveTo(QPointF(0, -s_stroke_width / 2));
    m_shape.lineTo(QPointF(s_wire_length, -s_stroke_width / 2));
    m_shape.lineTo(QPointF(s_wire_length, s_stroke_width / 2));
    m_shape.lineTo(QPointF(0, s_stroke_width / 2));
    m_shape.closeSubpath();

    qreal x = s_wire_length + s_arrow_offset;

    m_shape.moveTo(QPointF(x, 0));
    m_shape.lineTo(QPointF(x + s_arrow_left, s_arrow_height / 2));
    m_shape.lineTo(QPointF(x + s_arrow_left + s_arrow_length, s_arrow_height / 2));
    m_shape.lineTo(QPointF(x + s_arrow_left + s_arrow_length + s_arrow_right, 0));
    m_shape.lineTo(QPointF(x + s_arrow_left + s_arrow_length, -s_arrow_height / 2));
    m_shape.lineTo(QPointF(x + s_arrow_left, -s_arrow_height / 2));
    m_shape.closeSubpath();
}

void arrow_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_wires.append(mapped_position);

    qreal x = mapped_position.x();
    const qreal y = mapped_position.y();

    m_shape.moveTo(QPointF(x, y - s_stroke_width / 2));
    m_shape.lineTo(QPointF(x - s_wire_length, y - s_stroke_width / 2));
    m_shape.lineTo(QPointF(x - s_wire_length, y + s_stroke_width / 2));
    m_shape.lineTo(QPointF(x, y + s_stroke_width / 2));
    m_shape.closeSubpath();

    x -= s_wire_length + s_arrow_offset;

    m_shape.moveTo(QPointF(x, y));
    m_shape.lineTo(QPointF(x - s_arrow_right, y - s_arrow_height / 2));
    m_shape.lineTo(QPointF(x - s_arrow_right - s_arrow_length, y - s_arrow_height / 2));
    m_shape.lineTo(QPointF(x - s_arrow_right - s_arrow_length - s_arrow_left, y));
    m_shape.lineTo(QPointF(x - s_arrow_right - s_arrow_length, y + s_arrow_height / 2));
    m_shape.lineTo(QPointF(x - s_arrow_right, y + s_arrow_height / 2));
    m_shape.closeSubpath();
}

void arrow_separated_net::finalize()
{
    m_rect = m_shape.boundingRect();
    m_rect.adjust(-1, -1, 1, 1);
}

qreal arrow_separated_net::input_width() const
{  
    return s_input_width;
}

qreal arrow_separated_net::output_width() const
{
    return s_output_width;
}
