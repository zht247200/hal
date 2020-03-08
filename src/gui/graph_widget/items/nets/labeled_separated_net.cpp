#include "graph_widget/items/nets/labeled_separated_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

static const qreal s_baseline = 1;

qreal labeled_separated_net::s_wire_length;
qreal labeled_separated_net::s_text_offset;

QFont labeled_separated_net::s_font;
qreal labeled_separated_net::s_font_height;
qreal labeled_separated_net::s_font_ascend;

void labeled_separated_net::load_settings()
{
    s_wire_length = 20;
    s_text_offset = 2.4;

    s_font = QFont("Iosevka");
    s_font.setPixelSize(12);
    QFontMetricsF fm(s_font);
    s_font_height = fm.height();
    s_font_ascend = fm.ascent();
}

labeled_separated_net::labeled_separated_net(const std::shared_ptr<const net> n, const QString& text) : separated_graphics_net(n),
  m_text(text)
{
    QFontMetricsF fm(s_font);
    m_text_width = fm.width(m_text);
}

void labeled_separated_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    if (s_lod < graph_widget_constants::separated_net_min_lod)
        return;

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    color.setAlphaF(s_alpha);
    s_pen.setColor(color);
    painter->setPen(s_pen);
    painter->setFont(s_font);

    if (m_fill_icon)
    {
        s_brush.setColor(color);
        s_brush.setStyle(m_brush_style);
        painter->setBrush(s_brush); // ???
    }

    for (const QPointF& position : m_input_positions)
    {
        QPointF to(position.x() - s_wire_length, position.y());
        painter->drawLine(position, to);
        const qreal horizontal_offset = s_text_offset + m_text_width;
        const qreal vertical_offset = s_baseline + s_font_ascend - (s_font_height / 2);
        painter->drawText(QPointF(to.x() - horizontal_offset, to.y() + vertical_offset), m_text);

        if (m_fill_icon)
        {
            const QRectF rect(position.x() - s_wire_length - s_text_offset - m_text_width, position.y() - s_font_height / 2, m_text_width, s_font_height);
            painter->fillRect(rect, s_brush);
        }
    }

    for (const QPointF& position : m_output_positions)
    {
        QPointF to(position.x() + s_wire_length, position.y());
        painter->drawLine(position, to);
        const qreal vertical_offset = s_baseline + s_font_ascend - (s_font_height / 2);
        painter->drawText(QPointF(to.x() + s_text_offset, to.y() + vertical_offset), m_text);

        if (m_fill_icon)
        {
            const QRectF rect(position.x() + s_wire_length + s_text_offset, position.y() - s_font_height / 2, m_text_width, s_font_height);
            painter->fillRect(rect, s_brush);
        }
    }

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
    s_pen.setColor(Qt::green);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
#endif
}

void labeled_separated_net::add_input(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_input_positions.append(mapped_position);

    m_shape.moveTo(QPointF(mapped_position.x(), mapped_position.y() - s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - s_wire_length - s_text_offset, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() + QFontMetricsF(s_font).height()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + s_wire_length + s_text_offset, m_shape.currentPosition().y()));
    m_shape.closeSubpath();
}

void labeled_separated_net::add_output(const QPointF& scene_position)
{
    QPointF mapped_position = mapFromScene(scene_position);
    m_output_positions.append(mapped_position);

    m_shape.moveTo(QPointF(mapped_position.x(), mapped_position.y() - s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + s_wire_length + s_text_offset, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() + m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() + QFontMetricsF(s_font).height()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - m_text_width, m_shape.currentPosition().y()));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x(), m_shape.currentPosition().y() - QFontMetricsF(s_font).height() / 2 + s_shape_width / 2));
    m_shape.lineTo(QPointF(m_shape.currentPosition().x() - s_wire_length - s_text_offset, m_shape.currentPosition().y()));
    m_shape.closeSubpath();
}

qreal labeled_separated_net::input_width() const
{
    return s_wire_length + s_text_offset + m_text_width;
}

qreal labeled_separated_net::output_width() const
{
    return s_wire_length + s_text_offset + m_text_width;
}
