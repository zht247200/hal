#include "gui/netlist_model/netlist_proxy_model.h"

#include "gui/netlist_model/netlist_item.h"

netlist_proxy_model::netlist_proxy_model(QObject* parent) : QSortFilterProxyModel(parent)
{

}

bool netlist_proxy_model::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if(!filterRegExp().isEmpty())
    {
        //QModelIndex source_index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
        QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);
        if(source_index.isValid())
        {
            if (sourceModel()->data(source_index, filterRole()).toString().contains(filterRegExp()))
            {
                static_cast<netlist_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(true);
                return true;
            }
            else
            {
                static_cast<netlist_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(false);
                return false;
            }
        }
    }

    static_cast<netlist_item*>(sourceModel()->index(sourceRow, 0, sourceParent).internalPointer())->set_highlighted(false);
    return true;
    //return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
