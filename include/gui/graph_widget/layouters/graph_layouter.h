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

#ifndef GRAPH_LAYOUTER_H
#define GRAPH_LAYOUTER_H

#include "def.h"

#include "netlist/gate.h"
#include "netlist/net.h"

#include "gui/netlist_relay/netlist_relay.h"

#include <QObject>

class io_graphics_net;
class graph_context;
class graphics_gate;
class graphics_item;
class graphics_net;
class labeled_separated_net;
class standard_graphics_net;
class graphics_scene;

class graph_layouter : public QObject
{
    Q_OBJECT

public:
    explicit graph_layouter(QObject* parent = nullptr);
    virtual ~graph_layouter() = default;

    virtual QString name() const        = 0;
    virtual QString description() const = 0;

    virtual void layout() = 0;

    graphics_scene* scene() const;

Q_SIGNALS:
    void status_update(const int percent);
    void status_update(const QString& message);

protected:
    graphics_scene* m_scene;
};

#endif // GRAPH_LAYOUTER_H
