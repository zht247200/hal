#include "graph_widget/items/nets/standard_graphics_net.h"

#include "graph_widget/graph_widget_constants.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

#include <limits>

qreal standard_graphics_net::s_alpha;

qreal standard_graphics_net::s_wire_length;
qreal standard_graphics_net::s_left_arrow_offset;
qreal standard_graphics_net::s_right_arrow_offset;
qreal standard_graphics_net::s_arrow_left_x_shift;
qreal standard_graphics_net::s_arrow_right_x_shift;
qreal standard_graphics_net::s_arrow_side_length;
qreal standard_graphics_net::s_arrow_width;
qreal standard_graphics_net::s_arrow_height;

QPainterPath standard_graphics_net::s_arrow;

qreal standard_graphics_net::s_split_radius;

void standard_graphics_net::load_settings()
{
    s_wire_length = 26;

    s_left_arrow_offset = 3;
    s_right_arrow_offset = 3;

    s_arrow_left_x_shift = 0;
    s_arrow_right_x_shift = 3;
    s_arrow_side_length = 12;

    s_arrow_height = 6;
    s_arrow_width = s_arrow_left_x_shift + s_arrow_side_length + s_arrow_right_x_shift;

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

    s_split_radius = 3;
}

void standard_graphics_net::update_alpha()
{
    if (s_lod >= graph_widget_constants::net_fade_in_lod && s_lod <= graph_widget_constants::net_fade_out_lod)
        s_alpha = (s_lod - graph_widget_constants::net_fade_in_lod) / (graph_widget_constants::net_fade_out_lod - graph_widget_constants::net_fade_in_lod);
    else
        s_alpha = 1;
}

//standard_graphics_net::standard_graphics_net(const std::shared_ptr<const net> n, const lines& l) : graphics_net(n),
standard_graphics_net::standard_graphics_net(const std::shared_ptr<const net> n, lines& l) : graphics_net(n)
{    
    QVector<h_line> collapsed_h;
    QVector<v_line> collapsed_v;

    //for (const h_line& h : l.h_lines)
    for (h_line& h : l.h_lines)
    {
        if (h.small_x == h.big_x)
            continue;

        //assert(h.small_x < h.big_x);
        if (h.small_x > h.big_x)
        {
            qreal temp = h.small_x;
            h.small_x = h.big_x;
            h.big_x = temp;
        }

        QVector<int> overlaps;

        for (int i = 0; i < collapsed_h.size(); ++i)
            if (h.y == collapsed_h.at(i).y)
            {
                if (h.big_x < collapsed_h.at(i).small_x || collapsed_h.at(i).big_x < h.small_x)
                    continue; // NO OVERLAP

                // OVERLAP
                overlaps.append(i);
            }


        if (overlaps.isEmpty())
            collapsed_h.append(h);
        else
        {
            qreal smallest_x = h.small_x;
            qreal biggest_x = h.big_x;

            for (int i = 0; i < overlaps.size(); ++i)
            {
                int index = overlaps.at(i) - i;

                if (collapsed_h.at(index).small_x < smallest_x)
                    smallest_x = collapsed_h.at(index).small_x;

                if (collapsed_h.at(index).big_x > biggest_x)
                    biggest_x = collapsed_h.at(index).big_x;

                collapsed_h.remove(index);
            }

            collapsed_h.append(h_line{smallest_x, biggest_x, h.y});
        }
    }

    //for (const v_line& v : l.v_lines)
    for (v_line& v : l.v_lines)
    {
        if (v.small_y == v.big_y)
            continue;

        //assert(v.small_y < v.big_y);
        if (v.small_y > v.big_y)
        {
            qreal temp = v.small_y;
            v.small_y = v.big_y;
            v.big_y = temp;
        }

        QVector<int> overlaps;

        for (int i = 0; i < collapsed_v.size(); ++i)
            if (v.x == collapsed_v.at(i).x)
            {
                if (v.big_y < collapsed_v.at(i).small_y || collapsed_v.at(i).big_y < v.small_y)
                    continue; // NO OVERLAP

                // OVERLAP
                overlaps.append(i);
            }


        if (overlaps.isEmpty())
            collapsed_v.append(v);
        else
        {
            qreal smallest_y = v.small_y;
            qreal biggest_y = v.big_y;

            for (int i = 0; i < overlaps.size(); ++i)
            {
                int index = overlaps.at(i) - i;

                if (collapsed_v.at(index).small_y < smallest_y)
                    smallest_y = collapsed_v.at(index).small_y;

                if (collapsed_v.at(index).big_y > biggest_y)
                    biggest_y = collapsed_v.at(index).big_y;

                collapsed_v.remove(index);
            }

            collapsed_v.append(v_line{v.x, smallest_y, biggest_y});
        }
    }

    // CALCULATE SPLITS
    for (const h_line& h : collapsed_h)
    {
        for (const v_line& v : collapsed_v)
        {
            if (h.small_x <= v.x && v.x <= h.big_x)
                if (v.small_y < h.y && h.y < v.big_y)
                {
                    QPointF point(v.x, h.y);
                    m_splits.append(point);
                    m_shape.addEllipse(point, s_split_radius, s_split_radius);
                }

            if (v.small_y <= h.y && h.y <= v.big_y)
                if (h.small_x < v.x && v.x < h.big_x)
                {
                    QPointF point(v.x, h.y);
                    m_splits.append(point);
                    m_shape.addEllipse(point, s_split_radius, s_split_radius);
                }
        }
    }

    // CALCULATE RECT AND SHAPE
    qreal smallest_x = std::numeric_limits<qreal>::max();
    qreal biggest_x = std::numeric_limits<qreal>::min();

    qreal smallest_y = std::numeric_limits<qreal>::max();
    qreal biggest_y = std::numeric_limits<qreal>::min();

    for (const h_line& h : collapsed_h)
    {
        qreal small_x = h.small_x;
        qreal big_x = h.big_x;
        qreal y = h.y;

        if (small_x < smallest_x)
            smallest_x = small_x;

        if (big_x > biggest_x)
            biggest_x = big_x;

        if (y < smallest_y)
            smallest_y = y;
        else if (y > biggest_y)
            biggest_y = y;

        QLineF line(small_x, y, big_x, y);
        m_other_lines.append(line);
        QRectF rect(small_x - s_shape_width / 2, y - s_shape_width / 2, big_x - small_x + s_line_width + s_shape_width, s_line_width + s_shape_width);
        m_shape.addRect(rect);
    }

    for (const v_line& v : collapsed_v)
    {
        qreal x = v.x;
        qreal small_y = v.small_y;
        qreal big_y = v.big_y;

        if (x < smallest_x)
            smallest_x = x;
        else if (x > biggest_x)
            biggest_x = x;

        if (small_y < smallest_y)
            smallest_y = small_y;

        if (big_y > biggest_y)
            biggest_y = big_y;

        QLineF line(x, small_y, x, big_y);
        m_other_lines.append(line);
        QRectF rect(x - s_shape_width / 2, small_y - s_shape_width / 2, s_line_width + s_shape_width, big_y - small_y + s_line_width + s_shape_width);
        m_shape.addRect(rect);
    }


    const qreal padding = s_split_radius + s_shape_width;
    m_rect = QRectF(smallest_x - padding, smallest_y - padding, biggest_x - smallest_x + padding, biggest_y - smallest_y + padding);
}

void standard_graphics_net::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);

    QColor color = (option->state & QStyle::State_Selected) ? s_selection_color : m_color;
    s_pen.setColor(color);
    s_pen.setStyle(m_pen_style);

    painter->setPen(s_pen);
    painter->drawLines(m_other_lines);

    if (s_lod > graph_widget_constants::net_fade_in_lod)
    {
        color.setAlphaF(s_alpha);

        s_pen.setColor(color);
        s_pen.setStyle(Qt::SolidLine);
        painter->setPen(s_pen);

        s_brush.setColor(color);
        s_brush.setStyle(Qt::SolidPattern);
        painter->setBrush(s_brush);

        const bool original_antialiasing = painter->renderHints().testFlag(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::Antialiasing, true);

        for (const QPointF& point : m_splits)
            painter->drawEllipse(point, s_split_radius, s_split_radius);

        painter->setRenderHint(QPainter::Antialiasing, original_antialiasing);
    }

    s_brush.setStyle(Qt::NoBrush);
    painter->setBrush(s_brush);

#ifdef HAL_DEBUG_GUI_GRAPH_WIDGET
    s_pen.setColor(Qt::green);
    const bool original_cosmetic = s_pen.isCosmetic();
    s_pen.setCosmetic(true);
    painter->setPen(s_pen);
    painter->drawPath(m_shape);
    s_pen.setCosmetic(original_cosmetic);
#endif
}
