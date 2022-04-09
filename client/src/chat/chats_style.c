#include "chats.h"

void user_chats_add_style(Store *store) {
    GtkStyleContext *ctx = NULL;

    chat_entry *chat;

    GtkWidget *main_chat_box = GTK_WIDGET(gtk_builder_get_object(store->builder, "AllUserChats"));
    ctx = gtk_widget_get_style_context(GTK_WIDGET(main_chat_box));
    gtk_style_context_add_class(ctx, "MainChatBox");

    for (int i = 0; i < store->chats->chats_count; i++) {
        chat = store->chats->chats_arr[i];

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->gtk_chat_box));
        gtk_style_context_add_class(ctx, "ChatBox");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->gtk_chat_name));
        gtk_style_context_add_class(ctx, "ChatName");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->gtk_last_message));
        gtk_style_context_add_class(ctx, "ChatLastMessage");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->active_entry->chat_info_box));
        gtk_style_context_add_class(ctx, "ChatInfoBox");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->active_entry->chat_info_chat_name));
        gtk_style_context_add_class(ctx, "ChatInfoName");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->active_entry->chat_info_chat_bio));
        gtk_style_context_add_class(ctx, "ChatInfoBio");

        ctx = gtk_widget_get_style_context(GTK_WIDGET(chat->active_entry->chat_info_sec_label));
        gtk_style_context_add_class(ctx, "ChatInfoSecLabel");

        if (chat->active_entry->save_button)
            gtk_widget_set_classname(GTK_WIDGET(chat->active_entry->save_button), "ChatInfoSaveBtn");
    }
}

void members_add_style(chat_entry *chat) {
    GtkStyleContext *box_ctx = gtk_widget_get_style_context(chat->active_entry->members_box);
    gtk_style_context_add_class(box_ctx, "ChatInfoMembersBox");

    GtkStyleContext *members_label_ctx = gtk_widget_get_style_context(chat->active_entry->members_label);
    gtk_widget_set_halign(chat->active_entry->members_label, GTK_ALIGN_START);
    gtk_style_context_add_class(members_label_ctx, "ChatInfoMembersLabel");

    GtkStyleContext *member_ctx = NULL;
    for (int i = 0; i < chat->count_participants; i++) {
        member_ctx = gtk_widget_get_style_context(chat->active_entry->members_ui[i]->member_box);
        gtk_style_context_add_class(member_ctx, "ChatInfoMember");

        member_ctx = gtk_widget_get_style_context(chat->active_entry->members_ui[i]->name_label);
        gtk_style_context_add_class(member_ctx, "ChatInfoMemberLabel");
    }
}
