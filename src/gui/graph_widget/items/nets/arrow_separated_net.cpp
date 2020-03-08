#include "gui/graph_widget/items/nets/arrow_separated_net.h"

#include "gui/graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal arrow_separated_net::s_wire_length;
qreal arrow_separated_net::s_input_arrow_offset;
qreal arrow_separated_net::s_output_arrow_offset;
qreal arrow_separated_net::s_arrow_left_x_shift;
qreal arrow_separated_net::s_arrow_right_x_shift;
qreal arrow_separated_net::s_arrow_side_length;
qreal arrow_separated_net::s_arrow_width;
qreal arrow_separated_net::s_arrow_height;

qreal arrow_separated_net::s_input_width;
qreal arrow_separated_net::s_output_width;

QPainterPath arrow_separated_net::s_arrow;

void arrow_separated_net::load_settings()
{
    s_wire_length = 26;

    s_input_arrow_offset = 3;
    s_output_arrow_offset = 3;

    s_arrow_left_x_shift = 0;
    s_arrow_right_x_shift = 3;
    s_arrow_side_length = 12;

    s_arrow_height = 6;
    s_arrow_width = s_arrow_left_x_shift + s_arrow_side_length + s_arrow_right_x_shift;

    s_input_width = s_wire_length + s_input_arrow_offset + s_arrow_width + s_shape_width;
    s_output_width = s_wire_length + s_output_arrow_offset + s_arrow_width + s_shape_width;

    QPointF point(s_arrow_left_x_shift, -s_arrow_height / 2);

    s_arrow.clear();
    s_arrow.lineTo(point);
    point.setX(point.x() + s_arrow_side_length);
    s_arrow.lineTo(point);
    point.setX(point.x() + s_arrow_right_x_shift);
    point.setY(0);
    s_arrow.lineTo(point);
    point.setX(point.x() - s_arrow_right_x_shift);
    point.setY(s_arrow_height / 2);
    s_arrow.lineTo(point);
    point.setX(point.x() - s_arrow_side_length);
    s_arrow.lineTo(point);
    s_arrow.closeSubpath();
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
        s_brush.setStyle(m_brush_style);
        painter->setBrush(s_brush);
    }

    const Qt::PenStyle original_pen_style = s_pen.style();

    for (const QPointF& position : m_input_positions)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        s_pen.setStyle(Qt::PenStyle::SolidLine);
        painter->setPen(s_pen);
        painter->translate(QPointF(position.x() - s_wire_length - s_input_arrow_offset - s_arrow_width, position.y()));
        painter->drawPath(s_arrow);
        s_pen.setStyle(original_pen_style);
        painter->restore();
    }

    for (const QPointF& position : m_output_positions)
    {
        QPointF to(position.x() + s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        s_pen.setStyle(Qt::PenStyle::SolidLine);
        painter->setPen(s_pen);
        painter->translate(QPointF(position.x() + s_wire_length + s_output_arrow_offset, position.y()));
        painter->drawPath(s_arrow);
        s_pen.setStyle(original_pen_style);
        painter->restore();
    }

    painter->setBrush(QBrush());

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
    bool original_cosmetic = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(original_cosmetic);
#endif
}

void arrow_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_positions.append(mapped_position);

    const qreal half_of_shape_width = s_shape_width / 2;

    QPointF point(mapped_position.x() - s_wire_length - half_of_shape_width, mapped_position.y() - half_of_shape_width);

    m_shape.moveTo(point);
    point.setX(point.x() + s_wire_length + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_wire_length - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();

    point.setX(mapped_position.x() - s_wire_length - s_input_arrow_offset - s_arrow_width - half_of_shape_width);
    point.setY(mapped_position.y() - s_arrow_height / 2 - half_of_shape_width);

    m_shape.moveTo(point);
    point.setX(point.x() + s_arrow_width + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_arrow_height + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_arrow_width - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();
}

void arrow_separated_net::add_output(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_output_positions.append(mapped_position);

    const qreal half_of_shape_width = s_shape_width / 2;

    QPointF point(mapped_position.x() - half_of_shape_width, mapped_position.y() - half_of_shape_width);

    m_shape.moveTo(point);
    point.setX(point.x() + s_wire_length + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_wire_length - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();

    point.setX(mapped_position.x() + s_wire_length + s_output_arrow_offset - half_of_shape_width);
    point.setY(mapped_position.y() - s_arrow_height / 2 - half_of_shape_width);

    m_shape.moveTo(point);
    point.setX(point.x() + s_arrow_width + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_arrow_height + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_arrow_width - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();
}

qreal arrow_separated_net::input_width() const
{  
    return s_input_width;
}

qreal arrow_separated_net::output_width() const
{
    return s_output_width;
}
