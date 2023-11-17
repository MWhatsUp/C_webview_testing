#include "libs/webview/webview.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif


#define UNUSED(x) (void)x

typedef struct {
  webview_t w;
  unsigned int count;
} context_t;


void call_c(const char *seq, const char *req, void *arg) {
  UNUSED(req);
  context_t *context = (context_t *)arg;
  
  webview_eval(context->w, "write('Test from C')");

  webview_return(context->w, seq, 0, "");
}


#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
  (void)hInst;
  (void)hPrevInst;
  (void)lpCmdLine;
  (void)nCmdShow;
#else
int main(int argc, char *argv[]) {
#endif

  // Open the HTML file
  FILE *file = fopen("bind.html", "r");

  // Check if the file was opened successfully
  if (file == NULL) {
      perror("Error opening file");
      return 1; // Exit with an error code
  }

  // Get the file size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Allocate memory for the string
  char *html_content = (char *)malloc(file_size + 1);

  // Check if memory allocation was successful
  if (html_content == NULL) {
      perror("Error allocating memory");
      fclose(file);
      return 1; // Exit with an error code
  }

  // Read the file content into the string
  size_t read_size = fread(html_content, 1, file_size, file);

  // Check if the read was successful
  if (read_size != file_size) {
      perror("Error reading file");
      fclose(file);
      free(html_content);
      return 1; // Exit with an error code
  }

  // Null-terminate the string
  html_content[file_size] = '\0';

  // Close the file
  fclose(file);



  webview_t w = webview_create(0, NULL);
  context_t context = {.w = w};
  webview_set_title(w, "Basic Example");
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);

  webview_bind(w, "call_c", call_c, &context);

  webview_set_html(w, html_content);
  webview_run(w);
  webview_destroy(w);
  // system("mkdir haha");


   // Free allocated memory
  free(html_content);
  return 0;
}
