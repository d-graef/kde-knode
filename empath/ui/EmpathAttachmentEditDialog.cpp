/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathAttachmentEditDialog.h"
#endif

#include <qwhatsthis.h>

// KDE includes
#include <kfiledialog.h>
#include <klocale.h>
#include <kapp.h>

// Local includes
#include "EmpathUIUtils.h"
#include "EmpathDefines.h"
#include "EmpathAttachmentEditDialog.h"

    const QString
EmpathAttachmentEditDialog::charsetTypes_[] = { 
    "us-ascii (English)",
    "iso-8859-1 (Latin-1)",
    "iso-8859-2 (Latin-2)",
    "iso-8859-3 (Esperanto)",
    "iso-8859-4 (Baltic)",
    "iso-8859-5 (Cyrillic)",
    "iso-8859-6 (Arabic)",
    "iso-8859-7 (Greek)",
    "iso-8859-8 (Hebrew)",
    "iso-8859-9 (Turkish)",
    "iso-8859-10 (Nordic)",
    "KOI-8R (Russian)"
};

    const int
EmpathAttachmentEditDialog::nChr = 12;

    const QString
EmpathAttachmentEditDialog::textSubTypes_[] = {
    "Plain",
    "RichText",
    "HTML"
};
    const int
EmpathAttachmentEditDialog::nTxt = 3;

    const QString
EmpathAttachmentEditDialog::messageSubTypes_[] = {
    "RFC822",
    "Digest",
    "Parallel",
    "Partial",
    "External-Body"
};    
    const int
EmpathAttachmentEditDialog::nMsg = 5;

    const QString
EmpathAttachmentEditDialog::applicationSubTypes_[] = {
    "Octet-Stream",
    "X-cpio",
    "X-DVI",
    "X-perl",
    "X-tar",
    "X-deb",
    "X-rar-compressed",
    "X-LaTeX",
    "X-sh",
    "X-shar",
    "X-tar-gz",
    "X-tcl",
    "X-TeX",
    "X-troff",
    "X-zip",
    "X-VRML"
};

    const int
EmpathAttachmentEditDialog::nApp = 16;

    const QString
EmpathAttachmentEditDialog::imageSubTypes_[] = {
    "JPEG",
    "GIF",
    "PNG",
    "TIFF",
    "X-XBitmap",
    "X-XPixmap",
    "X-CMU-Raster",
    "X-Portable-Anymap",
    "X-Portable-Bitmap",
    "X-Portable-Graymap",
    "X-Portable-Pixmap",
    "X-RGB"
};

    const int
EmpathAttachmentEditDialog::nImg = 12; 

    const QString
EmpathAttachmentEditDialog::videoSubTypes_[] = {
    "MPEG",
    "QuickTime",
    "FLI",
    "GL",
    "X-SGI-Movie",
    "X-MSVideo"
};

    const int
EmpathAttachmentEditDialog::nVid = 6;

    const QString
EmpathAttachmentEditDialog::audioSubTypes_[] = {
    "MIDI",
    "ULAW",
    "X-AIFF",
    "X-WAV"
};

    const int
EmpathAttachmentEditDialog::nAud = 4;

EmpathAttachmentEditDialog::EmpathAttachmentEditDialog(
        QWidget * parent,
        const char * name)
    :    QDialog(parent, name, true)
{
    setCaption(i18n("Attachment Edit"));
    
    KButtonBox * buttonBox    = new KButtonBox(this);

    // Bottom button group
    pb_help_    = buttonBox->addButton(i18n("&Help"));    
    buttonBox->addStretch();
    pb_OK_        = buttonBox->addButton(i18n("&OK"));
    pb_OK_->setDefault(true);
    pb_cancel_    = buttonBox->addButton(i18n("&Cancel"));
    
    buttonBox->layout();

    QObject::connect(pb_OK_,        SIGNAL(clicked()),  SLOT(s_OK())); 
    QObject::connect(pb_cancel_,    SIGNAL(clicked()),  SLOT(s_cancel()));
    QObject::connect(pb_help_,      SIGNAL(clicked()),  SLOT(s_help()));

    QLabel * l_filename    = new QLabel(i18n("Filename"), this, "l_filename");
    QLabel * l_description = new QLabel(i18n("Description"), this, "l_descrip");
    le_filename_   = new QLineEdit(this, "le_filename");
    
    QWhatsThis::add(le_filename_, i18n("Pick a file to attach !"));
    
    le_description_ = new QLineEdit(this, "le_description");
    
    QWhatsThis::add(le_description_, i18n(
            "Write your own description of the attachment here.\n"
            "For example, ``Here's that file you didn't want''"));
    
    bg_encoding_ =
        new QButtonGroup(this, "bg_encoding");

    bg_encoding_->hide();
    bg_encoding_->setExclusive(true);

    rb_base64_    =
        new QRadioButton(i18n("Base 64"), this, "rb_base64_");
    
    QWhatsThis::add(rb_base64_, i18n(
            "Encode the attachment using base 64.\n"
            "This is generally the best encoding type to use"));

    rb_qp_        =
        new QRadioButton(i18n("Quoted printable"), this, "rb_qp");
    
    QWhatsThis::add(rb_qp_, i18n(
            "Encode the attachment as quoted-printable.\n"
            "This is useful when you're not sure if your\n"
            "recipient is able to read 8 bit or base64 messages\n"
            "as it doesn't make text completely unreadable."));

    rb_8bit_    =
        new QRadioButton(i18n("8 Bit"), this, "rb_8bit_");
    
    QWhatsThis::add(rb_8bit_, i18n(
            "Encode the attachment using 8 bit.\n"
            "Actually, this doesn't do anything much\n"
            "and it's fine for just sending text."));

    rb_7bit_    =
        new QRadioButton(i18n("7 bit"), this, "rb_7bit_");
    
    QWhatsThis::add(rb_7bit_, i18n(
            "Encode the attachment as 7 bit.\n"
            "This is only useful if you want to strip\n"
            "any 8 bit characters as the message will\n"
            "be passing through an MTA that can't cope\n"
            "with 8 bit, or being read by a dumb MUA.\n"
            "If you don't understand this, don't worry."));

    rb_base64_    ->setChecked(true);
    rb_8bit_    ->setChecked(false);
    rb_7bit_    ->setChecked(false);
    rb_qp_        ->setChecked(false);
    
    bg_encoding_->insert(rb_base64_,    RMM::CteTypeBase64);
    bg_encoding_->insert(rb_8bit_,        RMM::CteType8bit);
    bg_encoding_->insert(rb_7bit_,        RMM::CteType7bit);
    bg_encoding_->insert(rb_qp_,        RMM::CteTypeQuotedPrintable);
    
    QObject::connect(
        bg_encoding_, SIGNAL(clicked(int)),
        this, SLOT(s_encodingChanged(int)));

    QLabel * l_type       = new QLabel(i18n("Type"), this, "l_type");

    QLabel * l_subType    = new QLabel(i18n("SubType"), this, "l_subType");
    
    cb_type_ = new QComboBox(this, "cb_type");
    
    QWhatsThis::add(cb_type_, i18n(
            "Specify the major type of the attachment."));
    
    cb_type_->insertItem("text",        0);
    cb_type_->insertItem("message",     1);
    cb_type_->insertItem("application", 2);
    cb_type_->insertItem("image",       3);
    cb_type_->insertItem("video",       4);
    cb_type_->insertItem("audio",       5);
    
    cb_subType_        = new QComboBox(true, this, "cb_subType");
    
    QWhatsThis::add(cb_subType_, i18n(
            "Specify the minor type of the attachment.\n"
            "You may make up your own here, but precede\n"
            "it with 'X-'."));

    QLabel * l_charset_ = new QLabel(i18n("Charset"), this, "l_charset");
    
    cb_charset_        = new QComboBox(this, "cb_charset");
    
    QWhatsThis::add(cb_charset_, i18n(
            "Choose the character set that this attachment\n"
            "will be specified to be using."));
    
    // Layouts

    QObject::connect(
        cb_type_, SIGNAL(activated(int)),
        this, SLOT(s_typeChanged(int)));
    
    _init();
}

EmpathAttachmentEditDialog::~EmpathAttachmentEditDialog()
{
    empathDebug("dtor");
}

    void
EmpathAttachmentEditDialog::_init()
{
    le_filename_->setText(spec_.filename());
    le_description_->setText(spec_.description());
    
    int i;
    
    i = 0; do txtST_.append(textSubTypes_       [i++]); while (i != nTxt);
    i = 0; do msgST_.append(messageSubTypes_    [i++]); while (i != nMsg);
    i = 0; do appST_.append(applicationSubTypes_[i++]); while (i != nApp);
    i = 0; do imgST_.append(imageSubTypes_      [i++]); while (i != nImg);
    i = 0; do vidST_.append(videoSubTypes_      [i++]); while (i != nVid);
    i = 0; do audST_.append(audioSubTypes_      [i++]); while (i != nAud);
    i = 0; do chrT_ .append(charsetTypes_       [i++]); while (i != nChr);
    
    s_typeChanged(2);
    cb_type_->setCurrentItem(2);
    
    bg_encoding_->setButton((int)(spec_.encoding()));
}

    void
EmpathAttachmentEditDialog::s_OK()
{
    accept();
}

    void
EmpathAttachmentEditDialog::s_cancel()
{
    reject();
}

    void
EmpathAttachmentEditDialog::s_help()
{
    // STUB
}

    void
EmpathAttachmentEditDialog::s_browse()
{
    QString filename = KFileDialog::getOpenFileName();
    
    if (filename.isEmpty() || filename.at(filename.length() - 1) == '/')
        return;

    le_filename_->setText(filename);
    
    int lastSlash = filename.findRev('/');

    if (lastSlash == -1) // eh ?
        return;
    
    le_description_->setText(filename.mid(lastSlash + 1));
}

    void
EmpathAttachmentEditDialog::s_encodingChanged(int i)
{
    spec_.setEncoding((RMM::CteType)i);
}

    void
EmpathAttachmentEditDialog::s_typeChanged(int idx)
{
    cb_subType_->clear();

    switch (idx) {
        
        case 0: cb_subType_->insertStringList(txtST_);  break;
        case 1: cb_subType_->insertStringList(msgST_);  break;
        case 2: cb_subType_->insertStringList(appST_);  break;
        case 3: cb_subType_->insertStringList(imgST_);  break;
        case 4: cb_subType_->insertStringList(vidST_);  break;
        case 5: cb_subType_->insertStringList(audST_);  break;
        default:                                        break;
    }
}

    EmpathAttachmentSpec
EmpathAttachmentEditDialog::spec()
{
    spec_.setFilename        (le_filename_->text());
    spec_.setDescription    (le_description_->text());
    spec_.setType            (cb_type_->currentText());
    spec_.setSubType        (cb_subType_->currentText());
    spec_.setCharset        (cb_charset_->currentText());

    return spec_;
}

    void
EmpathAttachmentEditDialog::setSpec(const EmpathAttachmentSpec & s)
{
    spec_ = s;
    
    bg_encoding_->setButton((int)(spec_.encoding()));
    
    le_filename_->setText(spec_.filename());
    le_description_->setText(spec_.description());


//    cb_type_->setCurrentItem(s.type_);
//    cb_subType_->setCurrentItem(s.subType_);
}

// vim:ts=4:sw=4:tw=78
