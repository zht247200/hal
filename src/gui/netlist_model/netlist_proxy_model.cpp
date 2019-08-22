#include "gui/netlist_model/netlist_proxy_model.h"

#include "gui/explorer_widget/explorer_widget.h"
#include "gui/netlist_model/netlist_item.h"

netlist_proxy_model::netlist_proxy_model(explorer_widget* parent) : QSortFilterProxyModel(parent),
    m_widget(parent)
{
    assert(parent);
}

bool netlist_proxy_model::filterAcceptsRow(const int sourceRow, const QModelIndex& sourceParent) const
{
    if(!filterRegExp().isEmpty())
    {
        QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);

        if(source_index.isValid()) // NECESSARY ???
        {
            netlist_item* item = static_cast<netlist_item*>(source_index.internalPointer());

            switch (item->type())
            {
            case hal::item_type::module: break;
            case hal::item_type::gate:
            {
                if (m_widget->show_gates())
                    break;
                else
                {
                    item->set_highlighted(false);
                    return false;
                }
            }
            case hal::item_type::net:
            {
                if (m_widget->show_nets())
                    break;
                else
                {
                    item->set_highlighted(false);
                    return false;
                }
            }
            }

            if (sourceModel()->data(source_index, filterRole()).toString().contains(filterRegExp()))
            {
                item->set_highlighted(true);
                return true;
            }
            else
            {
                item->set_highlighted(false);
                return false;
            }
        }
    }

    static_cast<netlist_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(false);
    return true;
}
