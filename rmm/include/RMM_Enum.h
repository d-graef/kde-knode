/*
	Empath - Mailer for KDE

	Copyright (C) 1998 Rik Hemsley rik@kde.org

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

#ifndef RMM_ENUM_H
#define RMM_ENUM_H

#include <qstring.h>
#include <qcstring.h>

class RMM
{
	public:

		enum MessageStatus {
			Read		= 1 << 0,
			Marked		= 1 << 1,
			Trashed		= 1 << 2,
			Replied		= 1 << 3
		};

		static const char * monthNames[];

		static const char * headerNames[];

		enum HeaderType {
			HeaderApproved,
			HeaderBcc,
			HeaderCc,
			HeaderComments,
			HeaderContentDescription,
			HeaderContentDisposition,
			HeaderContentID,
			HeaderContentMD5,
			HeaderContentType,
			HeaderControl,
			HeaderContentTransferEncoding,
			HeaderDate,
			HeaderDistribution,
			HeaderEncrypted,
			HeaderExpires,
			HeaderFollowupTo,
			HeaderFrom,
			HeaderInReplyTo,
			HeaderKeywords,
			HeaderLines,
			HeaderMessageID,
			HeaderMimeVersion,
			HeaderNewsgroups,
			HeaderOrganization,
			HeaderPath,
			HeaderReceived,
			HeaderReferences,
			HeaderReplyTo,
			HeaderResentBcc,
			HeaderResentCc,
			HeaderResentDate,
			HeaderResentFrom,
			HeaderResentMessageID,
			HeaderResentReplyTo,
			HeaderResentSender,
			HeaderResentTo,
			HeaderReturnPath,
			HeaderSender,
			HeaderSubject,
			HeaderSummary,
			HeaderTo,
			HeaderXref,
			HeaderUnknown
		};

		enum HeaderDataType {
			Address,
			AddressList,
			ContentType,
			Cte,
			DateTime,
			DispositionType,
			Mailbox,
			MailboxList,
			Mechanism,
			MessageID,
			Text
		};

		static const HeaderDataType headerTypesTable[];

		enum CteType {
			CteType7bit,
			CteType8bit,
			CteTypeBinary,
			CteTypeQuotedPrintable,
			CteTypeBase64,
			CteTypeXtension
		};

		enum MimeType {
			MimeTypeNone,
			MimeTypeUnknown,
			MimeTypeText,
			MimeTypeMultiPart,
			MimeTypeMessage,
			MimeTypeApplication,
			MimeTypeImage,
			MimeTypeAudio,
			MimeTypeVideo,
			MimeTypeModel
		};

		enum MimeSubType {
			MimeSubTypeNone,
			MimeSubTypeUnknown,
			MimeSubTypePlain,
			MimeSubTypeRichtext,
			MimeSubTypeEnriched,
			MimeSubTypeHTML,
			MimeSubTypeMixed,
			MimeSubTypeAlternative,
			MimeSubTypeDigest,
			MimeSubTypeParallel,
			MimeSubTypeRFC822,
			MimeSubTypePartial,
			MimeSubTypeExternalBody,
			MimeSubTypePostScript,
			MimeSubTypeOctetStream,
			MimeSubTypeJpeg,
			MimeSubTypeGIF,
			MimeSubTypeBasic,
			MimeSubTypeMPEG,
			MimeSubTypeXcpio,
			MimeSubTypeXdvi,
			MimeSubTypeXperl,
			MimeSubTypeXtar,
			MimeSubTypeXdeb,
			MimeSubTypeXrar,
			MimeSubTypeXlatex,
			MimeSubTypeXsh,
			MimeSubTypeXshar,
			MimeSubTypeXtgz,
			MimeSubTypeXtcl,
			MimeSubTypeXtex,
			MimeSubTypeXtroff,
			MimeSubTypeXzip,
			MimeSubTypeXras,
			MimeSubTypeXpnm,
			MimeSubTypeXpbm,
			MimeSubTypeXpgm,
			MimeSubTypeXppm,
			MimeSubTypeXrgb,
			MimeSubTypeXxbm,
			MimeSubTypeXxpm,
			MimeSubTypeXavi,
			MimeSubTypeXsgi,
			MimeSubTypeXaiff,
			MimeSubTypeXwav,
			MimeSubTypePNG,
			MimeSubTypeTIFF,
			MimeSubTypeGL,
			MimeSubTypeFLI,
			MimeSubTypeQuickTime,
			MimeSubTypeXvrml,
			MimeSubTypeMIDI,
			MimeSubTypeULAW
		};

		enum DispType {
			DispositionTypeInline,
			DispositionTypeAttachment
		};

		enum DayOfWeek {
			DayNone,
			DayMon, DayTue, DayWed, DayThu, DayFri, DaySat, DaySun
		};

		// DayOfWeek strToDayOfWeek(const QCString & s);

		enum Month {
			MonthJan, MonthFeb, MonthMar, MonthApr, MonthMay, MonthJun,
			MonthJul, MonthAug, MonthSep, MonthOct, MonthNov, MonthDec
		};

		static	Month		strToMonth(const QCString & s);

		static	MimeType	mimeTypeStr2Enum(const QCString & s);
		static	QCString	mimeTypeEnum2Str(MimeType m);
		static	MimeSubType	mimeSubTypeStr2Enum(const QCString & s);
		static	QCString	mimeSubTypeEnum2Str(MimeSubType t);
		static	CteType		RCteStr2Enum(const QCString & s);
		static 	QCString	cteTypeEnumToStr(CteType t);

		static	QString		mimeTypeToIconName(MimeType t, MimeSubType st);
};

#endif
