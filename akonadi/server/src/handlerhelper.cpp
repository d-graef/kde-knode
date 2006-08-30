/***************************************************************************
 *   Copyright (C) 2006 by Tobias Koenig <tokoe@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "handlerhelper.h"

using namespace Akonadi;

QList<QByteArray> HandlerHelper::splitLine( const QByteArray &line )
{
  QList<QByteArray> retval;

  int i, start = 0;
  bool escaped = false;
  for ( i = 0; i < line.count(); ++i ) {
    if ( line[ i ] == ' ' ) {
      if ( !escaped ) {
        retval.append( line.mid( start, i - start ) );
        start = i + 1;
      }
    } else if ( line[ i ] == '"' ) {
      if ( escaped ) {
        escaped = false;
      } else {
        escaped = true;
      }
    }
  }

  retval.append( line.mid( start, i - start ) );

  return retval;
}

// ### FIXME massive code duplication with libakonadi/imapparser.cpp!
int HandlerHelper::parseQuotedString( const QByteArray &data, QByteArray &result, int start )
{
  int begin = stripLeadingSpaces( data, start );
  int end = begin;

  // quoted string
  if ( data[begin] == '"' ) {
    ++begin;
    for ( int i = begin; i < data.length(); ++i ) {
      if ( data[i] == '\\' ) {
        ++i;
        continue;
      }
      if ( data[i] == '"' ) {
        result = data.mid( begin, i - begin );
        end = i + 1; // skip the '"'
        break;
      }
    }
  }

  // unquoted string
  else {
    for ( int i = begin; i < data.length(); ++i ) {
      if ( data[i] == ' ' || data[i] == '(' || data[i] == ')' ) {
        result = data.mid( begin, i - begin );

        // transform unquoted NIL
        if ( result == "NIL" )
          result.clear();

        end = i;
        break;
      }
    }
  }

  // strip quotes
  while ( result.contains( "\\\"" ) )
    result.replace( "\\\"", "\"" );

  return end;
}

int HandlerHelper::stripLeadingSpaces( const QByteArray & data, int start )
{
  for ( int i = start; i < data.length(); ++i ) {
    if ( data[i] != ' ' )
      return i;
  }
  return data.length();
}

