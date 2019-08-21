//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#ifndef NETLIST_ITEM_H
#define NETLIST_ITEM_H

#include "def.h"

#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

class netlist_item
{
public:
    netlist_item(const QString& name, const u32 id);
    ~netlist_item();

    void insert_child(int row, netlist_item* child);
    void remove_child(netlist_item* child);

    netlist_item* parent();
    netlist_item* child(int row);

    const netlist_item* const_parent() const;
    const netlist_item* const_child(int row) const;

    int childCount() const;
    QVariant data(int column) const;
    int row() const;

    QString name() const;
    u32 id() const;
    QColor color() const;
    bool highlighted() const;

    void set_parent(netlist_item* parent);
    void set_name(const QString& name);
    void set_color(const QColor& color);
    void set_highlighted(const bool highlighted);

private:
    netlist_item* m_parent;
    QList<netlist_item*> m_child_items;

    QString m_name;
    u32 m_id;

    QColor m_color;
    bool m_highlighted;
};

#endif // NETLIST_ITEM_H
