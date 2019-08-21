/*
 * MIT License
 *
 * Copyright (c) 2019 Chair for Embedded Security, Ruhr-University Bochum
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef EXPLORER_WIDGET_H
#define EXPLORER_WIDGET_H

#include "gui/content_widget/content_widget.h"
#include "gui/selection_relay/selection_relay.h"

#include <QItemSelection>
#include <QObject>

class netlist_proxy_model;
class searchbar;

class QTreeView;

class explorer_widget : public content_widget
{
    Q_OBJECT

public:
    explorer_widget(QWidget* parent = nullptr);

    virtual void setup_toolbar(toolbar* toolbar) override;
    virtual QList<QShortcut*> create_shortcuts() override;

public Q_SLOTS:
    void toggle_searchbar();
    void filter(const QString& text);
    void handle_intern_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
    void handle_custom_context_menu_requested(const QPoint& point);

private:
    QTreeView* m_tree_view;
    searchbar* m_searchbar;

    netlist_proxy_model* m_netlist_proxy_model;

    bool m_ignore_selection_change;
};

#endif // EXPLORER_WIDGET_H
