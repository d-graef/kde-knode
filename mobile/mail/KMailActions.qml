/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (C) 2010 Andras Mantia <andras@kdab.net>

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

import Qt 4.7 as QML
import org.kde 4.5
import org.kde.pim.mobileui 4.5

ActionMenuContainer {

  actionItemHeight : height / 6 - actionItemSpacing
  actionItemWidth : 200
  actionItemSpacing : 2

  ActionList {
    category : "home"
    name : KDE.i18n( "Home" )
    FakeAction { name : "sync_all_emails" }
    FakeAction { name : "check_emails" }
    FakeAction { name : "send_queued_emails" }
    FakeAction { name : "select_multiple_folders" }
    FakeAction { name : "empty_all_trash_folders" }
  }

  ReorderList {
    category : "home"
    name : KDE.i18n( "Favorites" )

    delegate : QML.Component {
      QML.Text { height : 20; text : model.display }
    }
    upAction : "fav_up"
    downAction : "fav_down"
    deleteAction : "fav_delete"
    model : favoritesList
  }

  AgentInstanceList {
    category : "home"
    name : KDE.i18n( "Accounts" )

    model : agentInstanceList

    customActions : [
      ActionListItem { name : "akonadi_agentinstance_configure" },
      ActionListItem { name : "akonadi_agentinstance_delete" },
      ActionListItem { name : "akonadi_agentinstance_create" }
    ]
  }

  ActionListItem {
    category : "home"
    name : "kmail_mobile_identities"
  }

  ActionList {
    category : "account"
    name : KDE.i18n( "Account" )
    FakeAction { name : "check_mail_in_account" }
    FakeAction { name : "srv_side_subsrcp" }
    FakeAction { name : "local_subsrcp" }
    ActionListItem { name : "akonadi_resource_properties" }
    ActionListItem { name : "akonadi_collection_create" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Folder" )
    ActionListItem { name : "akonadi_collection_sync" }
    ActionListItem { name : "akonadi_mark_all_as"
                     argument : "R"
                     title : KDE.i18n("Mark All As Read")
                   }
    ActionListItem { name : "akonadi_move_all_to_trash" }
    ActionListItem { name : "akonadi_remove_duplicates" }
    FakeAction { name : "archive_folder" }
    FakeAction { name : "expire_rule" }
    FakeAction { name : "ml_management" }
    FakeAction { name : "empty_trash" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "Edit" )
    ActionListItem { name : "akonadi_collection_properties" }
    ActionListItem { name : "akonadi_collection_create" }
    ActionListItem { name : "akonadi_collection_move_to_menu" }
    ActionListItem { name : "akonadi_collection_copy_to_menu" }
    ActionListItem { name : "akonadi_collection_delete" }
  }

  ActionList {
    category : "single_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_as_favorite" }
    FakeAction { name : "start_maintaince" }
    FakeAction { name : "prefer_html_to_plain" }
    FakeAction { name : "load_external_ref" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "Folders" )
    FakeAction { name : "check_mail_in_sel_folder" }
    FakeAction { name : "mark_all_msg_as_read" }
    FakeAction { name : "move_all_msg_to_trash" }
    FakeAction { name : "remove_dupl_msgs" }
    FakeAction { name : "archive_selected_folders" }
  }

  ActionList {
    category : "multiple_folder"
    name : KDE.i18n( "View" )
    FakeAction { name : "save_as_favorite" }
    FakeAction { name : "start_maintaince" }
    FakeAction { name : "prefer_html_to_plain" }
    FakeAction { name : "load_external_ref" }
  }

  ActionList {
    category: "mail_viewer"
    name: KDE.i18n( "Email" )
    FakeAction { name : "reply" }
    FakeAction { name : "forward" }
    FakeAction { name : "mark_email_as" }
    FakeAction { name : "send_again" }
    FakeAction { name : "create_todo_reminder" }
    FakeAction { name : "find_in_email" }
    FakeAction { name : "save_as" }
  }

  ActionList {
    category: "mail_viewer"
    name: KDE.i18n( "Attachments" )
    FakeAction { name : "save_all" }
  }

  ActionList {
    category: "mail_viewer"
    name: KDE.i18n( "Edit" )
    FakeAction { name : "edit_email" }
    ActionListItem { name : "akonadi_item_copy_to_menu" }
    ActionListItem { name : "akonadi_item_move_to_menu" }
    ActionListItem { name : "akonadi_item_delete" }
  }

  ActionList {
    category: "standard"
    name: KDE.i18n( "Settings (merge)" )
    FakeAction { name : "work_offline" }
    FakeAction { name : "configure_notifications" }
  }

  ApplicationGeneralActions {
    category : "standard"
    name : KDE.i18n( "Application" )
    type : "mail"
  }
}
