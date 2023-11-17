#include "libs/webview/webview.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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


  int pipe_fd[2];
  pid_t pid;
  
  // Create a pipe
  if (pipe(pipe_fd) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
  }

  // Create a child process
  if ((pid = fork()) == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
  }

  if (pid == 0) {
      // Child process
      close(pipe_fd[0]); // Close unused read end of the pipe
      dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to the pipe
      close(pipe_fd[1]); // Close the write end of the pipe

      // Execute the command (replace "ls -l" with your desired command)
      execlp("ls", "ls", "-l", (char *)NULL);

      // If execlp fails
      perror("execlp");
      exit(EXIT_FAILURE);
  } else {
      // Parent process
      close(pipe_fd[1]); // Close the write end of the pipe

      // Read the output from the pipe
      char buffer[1024];
      ssize_t bytesRead;
      while ((bytesRead = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
          // write(STDOUT_FILENO, buffer, bytesRead);
          
          char text[bytesRead];
          strcpy(text, buffer);
          text[bytesRead] = '\0';

          char js[sizeof(text) + 9];
          sprintf(js, "write(`%s`)", text);

          webview_eval(context->w, js);
      }


      close(pipe_fd[0]); // Close the read end of the pipe

      // Wait for the child process to finish
      wait(NULL);
  }

  webview_return(context->w, seq, 0, "");
}

void exit_program(const char *seq, const char *req, void *arg) {
  exit(EXIT_SUCCESS);
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

  webview_bind(w, "exit_program", exit_program, &context);

  webview_set_html(w, html_content);
  webview_run(w);
  webview_destroy(w);
  // system("mkdir haha");


   // Free allocated memory
  free(html_content);
  return 0;
}
