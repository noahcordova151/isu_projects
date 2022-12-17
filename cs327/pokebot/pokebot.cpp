#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include <iostream>
#include <curl/curl.h>
#include <json-c/json.h>
#include <memory>
#include <cstring>

using namespace std;

void io_init_terminal(void)
{
  initscr();
  raw();
  curs_set(1);
  keypad(stdscr, TRUE);
}

/*
    Method to create and send a POST request to chatgpt API endpoint with given prompt string
    returns response string from chat_gpt 

    // DOESNT WORK!
*/
/*
string send_to_chatgpt(string user_prompt) {
    CURL *curl;   // handle
    CURLcode res; // return code
    struct curl_slist *headers = NULL;
    //char error[CURL_ERROR_SIZE];
    memory_t memory_chunk;
    string text_str;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        cerr << "curl initialization failure" << endl;
        return NULL;
    }

    string url = "https://api.openai.com/v1/completions";
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/completions");

    headers = curl_slist_append(headers, "Authorization: Bearer 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");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    //const char *user_prompt = "Say this is a test.";
    string json_str = "{\"model\": \"text-davinci-003\", \"prompt\": \"";
    json_str.append(user_prompt);
    json_str.append("\", \"temperature\": 0, \"max_tokens\": 7}");
    //string json_str = "{\"model\": \"text-davinci-003\", \"prompt\": \"Say this is a test.\", \"temperature\": 0, \"max_tokens\": 7}";

    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &memory_chunk);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);            // prints more data abt request to stdout
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);             // prints more data abt header to stdout
    //curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);   // prints more data abt error to stdout

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    // perform request, res will get return code        
    res = curl_easy_perform(curl);
    
    // check if curl_easy_perform failed
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        return NULL;
    } 

    // response info
    int httpCode(0);
    unique_ptr<string> httpData(new string());
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    // good response, parse json
    if (httpCode == 200)
    {
        cout << "\nGot successful response from " << url << endl;

        cout << "memory_chunk.memory:\n\n" << memory_chunk.memory << "\nend memory_chunk.memory" << endl;

        // Response looks good - done using Curl now.  Try to parse the results
        // and print them out.

        // find and store the string-ified json object from response string
        const char *json_object_str = strchr(memory_chunk.memory, '{');
        cout << "json_object_str: \n" << json_object_str << endl;

        // create json_object from json_object_string
        struct json_object *parsed_json;
        parsed_json = json_tokener_parse(json_object_str);

        struct json_object *choices_arr;    // json array containing one json object
        json_object_object_get_ex(parsed_json, "choices", &choices_arr);
        
        struct json_object *choices_obj;    // first json object from json array "choices"
        choices_obj = json_object_array_get_idx(choices_arr, 0);

        struct json_object *text;           // "text" field in "choices" json obj
        json_object_object_get_ex(choices_obj, "text", &text);

        text_str = json_object_get_string(text);
        cout << "text_str: " << text_str << endl;
    }
    else
    {
        cout << "Couldn't POST to " << url << ". Exiting" << endl;
        return NULL;
    }

    // cleanup
    free(memory_chunk.memory);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    headers = NULL;
    curl_global_cleanup();

    return text_str;
}
*/

// Define the callback function for handling the response
size_t handle_response(void *contents, size_t size, size_t nmemb, void *userp)
{
    // Convert the response to a string
    const size_t num_bytes = size * nmemb;
    char *response_str = (char *)malloc(num_bytes + 1);
    memcpy(response_str, contents, num_bytes);
    response_str[num_bytes] = '\0';

    // Parse the JSON response
    json_object *response_json = json_tokener_parse(response_str);

    // You can now access the data contained in the JSON response
    //  just like you would with any other json_object
    
    //json_object *object = json_object_object_get(response_json, "object");
    //printf("object: %s\n", json_object_get_string(object));
    
    //json_object *model = json_object_object_get(response_json, "model");
    //printf("model: %s\n", json_object_get_string(model));

    json_object *choices = json_object_object_get(response_json, "choices");
    //printf("choices: %s\n", json_object_get_string(choices));
    
    json_object *choice = json_object_array_get_idx(choices, 0);
    //printf("choice: %s\n", json_object_get_string(choice));

    json_object *text_completion = json_object_object_get(choice, "text");
    printw("AI: %s\n", json_object_get_string(text_completion));

    // Return the number of bytes processed
    return num_bytes;
}

int main (int argc, char *argv[]) {
    bool quit = 0;

    // init ncurses window
    io_init_terminal();

    mvprintw(0, 0, "Hi! I am PokeBot, your personal AI Assistant. Ask me anything about anything! \nHow can I help you today?");
    getch();

    do{
        clear();

        // prompt user for input
        mvprintw(0, 0, "Me: ");

        // get user input for prompt 
        char user_prompt[] = "";
        getstr(user_prompt);

        if (user_prompt[0] == 'q') {
            quit = 1;
            break;
        }

        move(1, 0);
        
        // Begin code for curl http POST Request + response ( in handle_response() )
        // vvv

        // Define the URL for the OpenAI chat.gpt completions API endpoint
        const char *url = "https://api.openai.com/v1/completions";

        // Define the API key to be used for authentication
        const string api_key = "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";

        // Define the JSON payload to be sent with the request
        json_object *payload = json_object_new_object();
        json_object_object_add(payload, "prompt", json_object_new_string(user_prompt));
        json_object_object_add(payload, "model", json_object_new_string("text-davinci-002"));
        json_object_object_add(payload, "max_tokens", json_object_new_int(50));
        json_object_object_add(payload, "temperature", json_object_new_double(1.0));

        // Convert the JSON payload to a string
        const char *json_payload = json_object_to_json_string(payload);

        // Initialize the curl library
        curl_global_init(CURL_GLOBAL_ALL);

        // Create a new curl handle
        CURL *curl = curl_easy_init();

        // Set the URL for the curl handle
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the request type to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1);

        // Set the JSON payload to be sent with the request
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);

        // Set the HTTP headers to be sent with the request
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        string api_key_header = "Authorization: Bearer " + api_key;
        headers = curl_slist_append(headers, api_key_header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the callback function for handling the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &handle_response);

        // Make the POST request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        // ^^^
        // End code for curl http POST Request + response ( in handle_response() )

        // leave on screen until user presses a button
        getch();
    } while (!quit);

    // end ncurses window
    endwin();

    return 0;
}