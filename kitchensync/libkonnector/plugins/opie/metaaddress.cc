
#include <kdebug.h>

#include "metaaddress.h"

using namespace OpieHelper;

namespace {
    bool test( const KABC::Addressee&,  const KABC::Addressee& );

};

MetaAddressReturn::MetaAddressReturn()
{

}
MetaAddressReturn::~MetaAddressReturn()
{

}
MetaAddressReturn::MetaAddressReturn( const MetaAddressReturn& ole )
{
    (*this) = ole;
}
QValueList<KABC::Addressee> MetaAddressReturn::added()
{
    return m_add;
}
QValueList<KABC::Addressee> MetaAddressReturn::modified()
{
    return m_mod;
}
QValueList<KABC::Addressee> MetaAddressReturn::removed()
{
    return m_rem;
}
MetaAddressReturn &MetaAddressReturn::operator=( const MetaAddressReturn& ole )
{
    m_add = ole.m_add;
    m_mod = ole.m_mod;
    m_rem = ole.m_rem;
    return *this;
}

MetaAddress::MetaAddress()
{

}
MetaAddress::~MetaAddress()
{

}
MetaAddressReturn MetaAddress::doMeta( const QValueList<KABC::Addressee>& newE,
                                       const QValueList<KABC::Addressee>& old )
{
    QValueList<KABC::Addressee> add;
    QValueList<KABC::Addressee> mod;
    QValueList<KABC::Addressee> rem;
    QValueList<KABC::Addressee>::ConstIterator it;
    QValueList<KABC::Addressee>::ConstIterator oldIt;
    bool found;
    for ( it = newE.begin(); it != newE.end(); ++it ) {
        found = false;
        for ( oldIt = old.begin(); oldIt != old.end(); ++oldIt ) {
            if ( (*oldIt).uid() == (*it).uid() ) {
                if ( test( (*it),  (*oldIt ) ) ) {
                    KABC::Addressee adr( (*it) ); // copy just in case
                    mod.append( adr );
                }
                found = true;
                break;
            }
        }
        if (!found ) {
            KABC::Addressee adr( (*it) );
            add.append( adr );
        }
    }
    for ( oldIt = old.begin(); oldIt != old.end(); ++oldIt ) {
        found = false;
        for ( it = newE.begin(); it != newE.end(); ++it ) {
            if ( (*it).uid() == (*oldIt).uid() ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            KABC::Addressee adr( (*oldIt) );
            rem.append( adr );
        }
    }
    MetaAddressReturn ret;
    ret.m_add = add;
    ret.m_mod = mod;
    ret.m_rem = rem;
    return ret;
};

namespace{

    bool test( const KABC::Addressee& ad1,  const KABC::Addressee& ad2 ) {
        kdDebug() << "ad1 " << ad1.givenName() << endl;
        bool ret = false;
        if ( ( !ad1.givenName().isEmpty() && !ad2.givenName().isEmpty() ) && ad1.givenName() != ad2.givenName() ) {
            kdDebug() << "Given Name mismatch" << endl;
            kdDebug() << "ad1 " << ad1.givenName() << " empty " << ad1.givenName().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.givenName() << " empty " << ad2.givenName().isEmpty() << endl;
            return true;
        }
        if ( (!ad1.additionalName().isEmpty() && !ad2.additionalName().isEmpty() ) && ad1.additionalName() != ad2.additionalName() ) {
            kdDebug() << "Additional name mismatch" << endl;
            kdDebug() << "ad1 " << ad1.additionalName() << " empty " << ad1.additionalName().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.additionalName() << " empty " << ad2.additionalName().isEmpty() << endl;
            return true;
        }
        if ( (!ad1.familyName().isEmpty() && !ad2.familyName().isEmpty() ) && ad1.familyName() != ad2.familyName() ) {
            kdDebug() << "Family Name mismatch "<< endl;
            kdDebug() << "ad1 " << ad1.familyName() << " empty " << ad1.familyName().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.familyName() << " empty " << ad2.familyName().isEmpty() << endl;
            return true;
        }
        if ( (!ad1.suffix().isEmpty() && !ad2.suffix().isEmpty() ) && ad1.suffix() != ad2.suffix() ) {
            kdDebug() << "Suffix mismatch " << endl;
            kdDebug() << "ad1 " << ad1.suffix() << " empty " << ad1.suffix().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.suffix() << " empty " << ad2.suffix().isEmpty() << endl;
            return true;
        }
        if ( (!ad1.sortString().isEmpty() && !ad2.sortString().isEmpty() ) && ad1.sortString() != ad2.sortString() ) {
            kdDebug() << "Sort string mismatch" << endl;
            kdDebug() << "ad1 " << ad1.sortString() << " empty " << ad1.sortString().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.sortString() << " empty " << ad2.sortString().isEmpty() << endl;
            return true;
        }
        if ( (!ad1.role().isEmpty() && !ad2.role().isEmpty() )&&  ad1.role() != ad2.role() ) {
            kdDebug() << "Rolse mismatch " << endl;
            kdDebug() << "ad1 " << ad1.role() << " empty " << ad1.role().isEmpty() << endl;
            kdDebug() << "ad2 " << ad2.role() << " empty " << ad2.role().isEmpty() << endl;
            return true;
        }
        QString custom1 = ad1.custom( "opie",  "Department");
        QString custom2 = ad2.custom( "opie",  "Department");
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 ) {
            kdDebug() << "Department mismatch " << endl;
            kdDebug() << "ad1 " << custom1 << endl;
            kdDebug() << "ad2 " << custom2 << endl;
            return true;
        }
        if ( (!ad1.organization().isEmpty() &&  !ad2.organization().isEmpty() ) && ad1.organization() != ad2.organization() ) {
            kdDebug() << "Organization mismatch" << endl;
            kdDebug() << "ad1 " << ad1.organization() << endl;
            kdDebug() << "ad2 " << ad2.organization() << endl;
            return true;
        }

        KABC::PhoneNumber phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work);
        KABC::PhoneNumber phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work);

        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() ) {
            kdDebug() << "Work Phone mismatch" << endl;
            return true;
        }

        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        if ((!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) &&  phone1.number() != phone2.number() )
            return true;

        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() )
            return true;
        if ( (!ad1.emails().isEmpty() && !ad2.emails().isEmpty() ) && ad1.emails() != ad2.emails() )
            return true;
        phone1 = ad1.phoneNumber( KABC::PhoneNumber::Home );
        phone2 = ad2.phoneNumber( KABC::PhoneNumber::Home );
        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell);
        phone2 = ad2.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell);
        if ( (!phone1.number().isEmpty() && !phone2.number().isEmpty()  ) && phone1.number() != phone2.number() )
            return true;

        KABC::Address adr1 = ad1.address( KABC::Address::Work );
        KABC::Address adr2 = ad2.address( KABC::Address::Work );
        if ( (!adr1.street().isEmpty() && !adr2.street().isEmpty() ) &&  adr1.street() != adr2.street() )
            return true;
        if ( (!adr1.locality().isEmpty() && !adr1.locality().isEmpty() ) && adr1.locality() != adr2.locality() )
            return true;
        if ( (!adr1.postalCode().isEmpty() && !adr2.postalCode().isEmpty() ) &&  adr1.postalCode() != adr2.postalCode() )
            return true;
        if ( (!adr1.country().isEmpty() && !adr2.country().isEmpty() ) && adr1.country() != adr2.country() )
            return true;
        if ( (!adr1.region().isEmpty() && !adr2.region().isEmpty() ) && adr1.region() != adr2.region() )
            return true;
        custom1 = ad1.custom( "opie",  "Office");
        custom2 = ad2.custom( "opie",  "Office");
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Profession" );
        custom2 = ad2.custom( "opie",  "Profession" );
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Assistant");
        custom2 = ad2.custom( "opie",  "Assistant");
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        adr1 = ad1.address( KABC::Address::Home );
        adr2 = ad1.address( KABC::Address::Home );
        if ( (!adr1.street().isEmpty() && !adr2.street().isEmpty() ) && adr1.street() != adr2.street() )
            return true;
        if ( (!adr1.locality().isEmpty() && !adr2.locality().isEmpty() ) && adr1.locality() != adr2.locality() )
            return true;
        if ( (!adr1.region().isEmpty() && !adr2.region().isEmpty() ) && adr1.region() != adr2.region() )
            return true;
        if ( (!adr1.postalCode().isEmpty() && !adr2.postalCode().isEmpty() ) && adr1.postalCode() != adr2.postalCode() )
            return true;
        if ( (!adr1.country().isEmpty() && !adr2.country().isEmpty() ) && adr1.country() != adr2.country() )
            return true;
        if ((!ad1.categories().isEmpty() && !ad2.categories().isEmpty() ) && ad1.categories() != ad2.categories() )
            return true;

        custom1 = ad1.custom( "opie", "HomeWebPage" );
        custom2 = ad2.custom( "opie", "HomeWebPage" );
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Spouse");
        custom2 = ad2.custom( "opie",  "Spouse");
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Gender");
        custom2 = ad2.custom( "opie",  "Gender");
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Birthday" );
        custom2 = ad2.custom( "opie",  "Birthday" );
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        custom1 = ad1.custom( "opie",  "Anniversary" );
        custom2 = ad2.custom( "opie",  "Anniversary" );
        if ( (!custom1.isEmpty() && !custom2.isEmpty() ) && custom1 != custom2 )
            return true;

        if ((!ad1.nickName().isEmpty() && ad2.nickName().isEmpty() ) && ad1.nickName() != ad2.nickName() )
            return true;
        if ( (!ad1.note().isEmpty() && !ad2.note().isEmpty() ) && ad1.note() != ad2.note() )
             return true;

        return ret;
    }
};
