/*
    This file is part of libkdepim.


    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPIM_ADDRESSEEDIFFALGO_H
#define KPIM_ADDRESSEEDIFFALGO_H

#include <kabc/addressee.h>
#include <libkdepim/diffalgo.h>
#include <QList>

namespace KPIM {

class KDEPIM_EXPORT AddresseeDiffAlgo : public DiffAlgo
{
  public:
    AddresseeDiffAlgo( const KABC::Addressee &leftAddressee, const KABC::Addressee &rightAddressee );

    void run();

  private:
    template <class L>
    void diffList( const QString &id, const QList<L> &left, const QList<L> &right );

    QString toString( const KABC::PhoneNumber &number );
    QString toString( const KABC::Address &address );

    KABC::Addressee mLeftAddressee;
    KABC::Addressee mRightAddressee;
};

}

#endif
