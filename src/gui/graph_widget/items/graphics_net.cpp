#include "gui/graph_widget/items/graphics_net.h"

#include "netlist/net.h"

#include <QPen>

QPen graphics_net::s_pen;

qreal graphics_net::s_pen_width; // STATIC CONST ?
qreal graphics_net::s_shape_spacing; // STATIC CONST ?

void graphics_net::load_settings()
{
    s_pen_width = 1.5;

    s_pen.setWidthF(s_pen_width);
    s_pen.setJoinStyle(Qt::MiterJoin);

    s_shape_spacing = s_pen_width / 2;
}

graphics_net::graphics_net(const std::shared_ptr<const net> n) : graphics_item(hal::item_type::net, n->get_id())
{
    assert(n);
}

QRectF graphics_net::boundingRect() const
{
    return m_rect;
}

QPainterPath graphics_net::shape() const
{
    return m_shape;
}
