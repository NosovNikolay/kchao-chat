#include "chats.h"

void apply_events(bson_t *updates_return) {
    bson_iter_t iter;
    if (!bson_iter_init_find(&iter, updates_return, "events") || !BSON_ITER_HOLDS_ARRAY(&iter))
        return;
    const bson_value_t *array = bson_iter_value(&iter);
    bson_t *updates = bson_new_from_data(array->value.v_doc.data, array->value.v_doc.data_len);

    bson_iter_t array_iter;
    if (bson_iter_init(&array_iter, updates)) {
        while (bson_iter_next(&array_iter)) {
            const bson_value_t *doc = bson_iter_value(&array_iter);
            bson_t *event = bson_new_from_data(doc->value.v_doc.data, doc->value.v_doc.data_len);
            string_t type = prepare_string(bson_get_str(event, "type"));

            if (mx_streqi(type, EVENT_TYPE_NEW_MESSAGE)) {
                bson_t *bson_message = bson_get_document(event, "message");
                message_t *message = message_new_from_bson(bson_message);
                bson_destroy(bson_message);
                g_idle_add(apply_message_event, (gpointer)message);
            } else if (mx_streqi(type, EVENT_TYPE_MESSAGE_DELETED)) {
            } else if (mx_streqi(type, EVENT_TYPE_MESSAGE_UPDATED)) {
            } else if (mx_streqi(type, EVENT_TYPE_GROUP_ADDED)) {
                bson_t *bson_chat = bson_get_document(event, "chat");
                chat_entry *chat = chat_new_from_bson(bson_chat);
                bson_destroy(bson_chat);
                chat->pack_position = get_age(chat->updated_at);
                g_idle_add(apply_chat_event, (gpointer)chat);
            } else if (mx_streqi(type, EVENT_TYPE_GROUP_REMOVED)) {
            } else if (mx_streqi(type, EVENT_TYPE_GROUP_CHANGED)) {
                bson_t *bson_chat = bson_get_document(event, "chat");
                chat_entry *chat = chat_new_from_bson(bson_chat);
                bson_destroy(bson_chat);
                
                Store *store = get_store();
                int chat_index = find_chat_index_by_id(store, chat->_id);
                if (chat_index != -1) {
                    
                    chat_entry *existed_chat = store->chats->chats_arr[chat_index];
        
                    existed_chat->name = mx_strdup(chat->name);

                    if (!is_private(existed_chat->type) && chat->avatar_id) {
                        existed_chat->avatar_id = mx_strdup(chat->avatar_id);
                    }
                    if (chat->description) {
                        existed_chat->description = mx_strdup(chat->description);
                    }
                    
                    existed_chat->participants_id = chat->participants_id;

                    existed_chat->updated_at = chat->updated_at;
                    
                    g_idle_add(apply_chat_changed_event, (gpointer)existed_chat);
                }

                
            } else if (mx_streqi(type, EVENT_TYPE_CHAT_DELETED)) {
            } else if (mx_streqi(type, EVENT_TYPE_USER_CHANGED)) {
            }

            free(type);
            bson_destroy(event);
        }
    }

    bson_destroy(updates);
}

gpointer run_updates(gpointer _) {
    Store *store = get_store();
    (void)_;
    int reconnect_timeout = 1;
    while (store->is_updatable) {
        RCResponse *res = api_get_updates();
        if (!res || res->code != SUCCESS) {
            sleep(reconnect_timeout);
            if (reconnect_timeout < 300) // 5 min is max reconnecting timeout
                reconnect_timeout *= 2;
            if (res)
                RC_response_destroy(res);
            continue;
        }
        reconnect_timeout = 1;
        printf("update responce: \n%s\n", res->body);

        bson_t *bson_update = NULL;
        bson_error_t bson_error;
        bson_update = bson_new_from_json(res->body, -1, &bson_error);
        if (!bson_update) {
            printf("Bson error %s\n", bson_error.message);
            continue;
        }
        if (store->is_updatable) {
            apply_events(bson_update);
        } else {
            bson_destroy(bson_update);
            RC_response_destroy(res);
            break;
        }

        bson_destroy(bson_update);
        RC_response_destroy(res);
        sleep(1);
    }
    return NULL;
}
