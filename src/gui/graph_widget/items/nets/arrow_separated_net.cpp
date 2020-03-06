#include "gui/graph_widget/items/nets/arrow_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal arrow_separated_net::s_wire_length;
qreal arrow_separated_net::s_input_arrow_offset;
qreal arrow_separated_net::s_output_arrow_offset;
qreal arrow_separated_net::s_arrow_length;
qreal arrow_separated_net::s_arrow_height;
qreal arrow_separated_net::s_arrow_left_outward_x_shift;
qreal arrow_separated_net::s_arrow_right_outward_x_shift;

qreal arrow_separated_net::s_input_offset;

qreal arrow_separated_net::s_input_width;
qreal arrow_separated_net::s_output_width;

QPainterPath arrow_separated_net::s_arrow;

QBrush arrow_separated_net::s_brush;

void arrow_separated_net::load_settings()
{
    s_wire_length = 26;
    s_input_arrow_offset = 3;
    s_output_arrow_offset = 3;
    s_arrow_length = 12;
    s_arrow_height = 6;
    s_arrow_left_outward_x_shift = 0;
    s_arrow_right_outward_x_shift = -3;

    s_input_offset = s_wire_length + s_input_arrow_offset + s_arrow_length;

    s_input_width = s_wire_length + s_input_arrow_offset + s_arrow_length + s_shape_width * 1.5;
    s_output_width = s_wire_length + s_output_arrow_offset + s_arrow_length + s_shape_width * 1.5;

    if (s_arrow_left_outward_x_shift > 0)
    {
        s_input_offset += s_arrow_left_outward_x_shift;

        s_input_width += s_arrow_left_outward_x_shift;
        s_output_width += s_arrow_left_outward_x_shift;
    }

    if (s_arrow_right_outward_x_shift < 0)
    {
        s_input_offset -= s_arrow_right_outward_x_shift;

        s_input_width -= s_arrow_right_outward_x_shift;
        s_output_width -= s_arrow_right_outward_x_shift;
    }

    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length, s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length - s_arrow_right_outward_x_shift, 0));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift + s_arrow_length, -s_arrow_height / 2));
    s_arrow.lineTo(QPointF(s_arrow_left_outward_x_shift, -s_arrow_height / 2));
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

    if (s_lod < graph_widget_constants::separated_net_min_lod)
        return;

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    color.setAlphaF(s_alpha);

    s_pen.setColor(color);
    painter->setPen(s_pen);

    if (m_fill_icon)
    {
        s_brush.setColor(color);
        painter->setBrush(s_brush);
    }

    for (const QPointF& position : m_input_positions)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(QPointF(position.x() - s_input_offset, position.y()));
        painter->drawPath(s_arrow);
        painter->restore();
    }

    for (const QPointF& position : m_output_positions)
    {
        QPointF to(position.x() + s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(QPointF(position.x() + s_wire_length + s_output_arrow_offset, position.y()));
        painter->drawPath(s_arrow);
        painter->restore();
    }

    painter->setBrush(QBrush());

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
    bool original_value = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(original_value);
#endif
}

void arrow_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_positions.append(mapped_position);

    const qreal half_of_shape_width = s_shape_width / 2;

    qreal x = mapped_position.x() + half_of_shape_width;
    const qreal y = mapped_position.y();

    m_shape.moveTo(QPointF(x, y - half_of_shape_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_shape_width, y - half_of_shape_width));
    m_shape.lineTo(QPointF(x - s_wire_length - s_shape_width, y + half_of_shape_width));
    m_shape.lineTo(QPointF(x, y + half_of_shape_width));
    m_shape.closeSubpath();

    x -= s_wire_length + s_input_arrow_offset;

    m_shape.moveTo(QPointF(x, y - s_arrow_height / 2 - half_of_shape_width));
    m_shape.lineTo(QPointF(x - s_arrow_length - s_shape_width, y - s_arrow_height / 2 - half_of_shape_width));
    m_shape.lineTo(QPointF(x - s_arrow_length - s_shape_width, y + s_arrow_height / 2 + half_of_shape_width));
    m_shape.lineTo(QPointF(x, y + s_arrow_height / 2 + half_of_shape_width));
    m_shape.closeSubpath();
}

void arrow_separated_net::add_output(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_output_positions.append(mapped_position);

    const qreal half_of_shape_width = s_shape_width / 2;

    qreal x = mapped_position.x() - half_of_shape_width;
    const qreal y = mapped_position.y();

    m_shape.moveTo(QPointF(x, y - half_of_shape_width));
    m_shape.lineTo(QPointF(x + s_wire_length + s_shape_width, y - half_of_shape_width));
    m_shape.lineTo(QPointF(x + s_wire_length + s_shape_width, y + half_of_shape_width));
    m_shape.lineTo(QPointF(x, y + half_of_shape_width));
    m_shape.closeSubpath();

    // ARROW
}

qreal arrow_separated_net::input_width() const
{  
    return s_input_width;
}

qreal arrow_separated_net::output_width() const
{
    return s_output_width;
}
