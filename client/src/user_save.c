#include "api/api.h"
#include "handlers/handlers.h"
#include "uchat.h"
#include "chat/chats.h"

int save_users(users_data *data, const char *filepath) {
    printf("save = %s\n", data->users->token[data->current_user]);
    printf("save = %s\n", data->users->user_id[data->current_user]);
    FILE *fp;
    if ((fp = fopen(filepath, "w")) == NULL)
    {
        perror("Error occured while opening file");
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    fwrite(&data->count_users, sizeof(int), 1, fp);
    fwrite(&data->current_user, sizeof(int), 1, fp);
    
    int str_len = 0;
    for (int i = 0; i < data->count_users; i++) {
        str_len = mx_strlen(data->users->user_id[i]);
        fwrite(&str_len, sizeof(int), 1, fp);
        fwrite(data->users->user_id[i], sizeof(char), str_len, fp);
        
        str_len = mx_strlen(data->users->token[i]);
        fwrite(&str_len, sizeof(int), 1, fp);
        fwrite(data->users->token[i], sizeof(char), str_len, fp);
    }
    

    fclose(fp);
    return 0;
}

users_data *init_data(int count_users) {
    users_data *data = malloc(sizeof(users_data));
    data->count_users = count_users;
    data->users = malloc(sizeof(User));
    data->users->user_id = malloc(sizeof(char *) * data->count_users);
    data->users->token = malloc(sizeof(char *) * data->count_users);

    return data;
}


users_data *load_users(const char *filepath) {
    FILE *fp;

    if ((fp = fopen(filepath, "rb")) == NULL) {
        mx_printerr("cant open binary file\n");
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    int result = 0;
    int count_users = -1;
    result = fread(&count_users, sizeof(int), 1, fp);
    if (result != 1) {
        mx_printerr("read data failed\n");
        return NULL;
    } 
    users_data *data = init_data(count_users);
    result = fread(&data->current_user, sizeof(int), 1, fp);
    if (result != 1) {
        mx_printerr("read data failed\n");
        return NULL;
    } 
    
    
    int str_size = 0;
    for (int i = 0; i < data->count_users; i++) {
        result = fread(&str_size, sizeof(int), 1, fp);
        
        if (result != 1) {
            mx_printerr("read data failed\n");
            return NULL;
        } 
        data->users->user_id[i] = mx_strnew(str_size);
        result = fread((void *)data->users->user_id[i], sizeof(char), str_size, fp);
        if (result != str_size) {
            mx_printerr("result != str_size\n");
            return NULL;
        }
        // TOKEN
        result = fread(&str_size, sizeof(int), 1, fp);
        
        if (result != 1) {
            mx_printerr("read data failed\n");
            return NULL;
        } 
        data->users->token[i] = mx_strnew(str_size);
        result = fread((void *)data->users->token[i], sizeof(char), str_size, fp);
        if (result != str_size) {
            mx_printerr("result != str_size\n");
            return NULL;
        } 

    }
    
    fclose(fp);
    return data;
}

bool is_user_in_data(users_data *data, const char* user_id){
    if (!data) return false;
    for (int i = 0; i < data->count_users; i++) {
        if (mx_streq(user_id, data->users->user_id[i])) return true;
    }
    return false;
}

users_data *add_user_to_data(users_data *data, const char* user_id, const char* token) {
    if (!data) {
        data = init_data(1);
        data->current_user = 0;
    }
    else {
        data->count_users++;
        data->users->token = realloc(data->users->token, sizeof(const char *) * data->count_users);
        data->users->token = realloc(data->users->user_id, sizeof(const char *) * data->count_users);
    }
    
    data->users->user_id[data->count_users - 1] = user_id;
    data->users->token[data->count_users - 1] = token;
    data->current_user = data->count_users - 1;
    
    return data;
}

void free_save_file(const char *filepath) {
    FILE *fp;
    if ((fp = fopen(filepath, "w")) == NULL)
    {
        perror("Error occured while opening file");
        return;
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
}


void free_user_data(users_data *data) {
    for (int i = 0; i < data->count_users; i++) {
        free(&data->users->token[i]);
        free(&data->users->user_id[i]);
    }
    free(data->users);
    free(data);
    data = NULL;
}
