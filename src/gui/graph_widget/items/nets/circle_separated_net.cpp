#include "graph_widget/items/nets/circle_separated_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <assert.h>

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

qreal circle_separated_net::s_wire_length;
qreal circle_separated_net::s_circle_offset;
qreal circle_separated_net::s_radius;

QBrush circle_separated_net::s_brush;

void circle_separated_net::load_settings()
{
    s_wire_length   = 26;
    s_circle_offset = 0;
    s_radius        = 3;

    s_brush.setStyle(Qt::SolidPattern);
    s_pen.setColor(QColor(160, 160, 160)); // USE STYLESHEETS
}

circle_separated_net::circle_separated_net(const std::shared_ptr<const net> n) : separated_graphics_net(n)
{
}

void circle_separated_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

    const bool original_antialiasing = painter->renderHints() & QPainter::Antialiasing;

    for (const QPointF& position : m_input_positions)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(QPointF(to.x() - s_circle_offset, to.y()), s_radius, s_radius);
        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    }

    for (const QPointF& position : m_output_positions)
    {
        QPointF to(position.x() + s_wire_length, position.y());
        painter->drawLine(position, to);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->drawEllipse(QPointF(to.x() + s_wire_length + s_circle_offset, to.y()), s_radius, s_radius);
        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    }

    painter->setBrush(QBrush());

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
    bool original_cosmetic = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPath(m_shape);
    painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    s_pen.setCosmetic(original_cosmetic);
#endif
}

void circle_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_positions.append(mapped_position);

    QPointF point(mapped_position.x() -s_wire_length - s_shape_width, mapped_position.y() -s_shape_width / 2);

    m_shape.moveTo(point);
    point.setX(point.x() + s_wire_length + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_wire_length - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();

    point.setX(mapped_position.x() - s_wire_length - s_circle_offset);
    point.setY(mapped_position.y());
    const qreal radius = s_radius + s_shape_width / 2;

    m_shape.addEllipse(point, radius, radius);
}

void circle_separated_net::add_output(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_output_positions.append(mapped_position);

    QPointF point(mapped_position.x() - s_shape_width, mapped_position.y() -s_shape_width / 2);

    m_shape.moveTo(point);
    point.setX(point.x() + s_wire_length + s_shape_width);
    m_shape.lineTo(point);
    point.setY(point.y() + s_shape_width);
    m_shape.lineTo(point);
    point.setX(point.x() - s_wire_length - s_shape_width);
    m_shape.lineTo(point);
    m_shape.closeSubpath();

    point.setX(mapped_position.x() + s_wire_length + s_circle_offset);
    point.setY(mapped_position.y());
    const qreal radius = s_radius + s_shape_width / 2;

    m_shape.addEllipse(point, radius, radius);
}

qreal circle_separated_net::input_width() const
{
    return s_wire_length + s_circle_offset + s_radius + s_line_width / 2;
}

qreal circle_separated_net::output_width() const
{
    return s_wire_length + s_circle_offset + s_radius + s_line_width / 2;
}
