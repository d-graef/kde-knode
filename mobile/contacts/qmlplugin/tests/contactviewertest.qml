/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

import Qt 4.6
import org.kde.akonadi.contacts 4.5

ContactView {
  x: 0
  y: 0
  width: 800
  height: 320
  contactItemId: 65906 
  transformOrigin: Item.TopLeft
//  SequentialAnimation on scale {
//    NumberAnimation { to: 1.05; duration: 2500; from: 0.95; easing.type: "InBounce" }
//    NumberAnimation { to: 0.95; from: 1.05; duration: 2500; easing.type: "OutBounce" }
//    //repeat: true;
//  }
}
