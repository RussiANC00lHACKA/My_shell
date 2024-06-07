#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

char *prompt = "my_shell> "; // Переменная для хранения пользовательского приглашения
void set_prompt(const char *new_prompt) {
    // Выделяем память для нового приглашения и добавляем символ '>'
    size_t length = strlen(new_prompt) + 2; // +2 для символов '>' и '\0'
    prompt = (char *)malloc(length);
    snprintf(prompt, length, "%s>", new_prompt); // Формируем новую строку с '>'
}
void reset_prompt() {
    prompt = strdup("my_shell> ");
    printf("Prompt reset to default.\n");
}
void set_all_style(const char *style) {
    if (strcmp(style, "bold") == 0) {
        printf("\033[1m"); // Жирный текст
    } else if (strcmp(style, "normal") == 0) {
        printf("\033[0m"); // Обычный текст
    } else if (strcmp(style, "small") == 0) {
        // Использование маленьких символов Unicode для имитации маленького текста
        printf("\033[1m"); // Устанавливаем жирный текст для лучшего восприятия
        printf("\u1D00"); // Пример маленького символа Unicode (можно добавить больше символов)
    }
}
void set_text_style(const char *style, const char *text) {
    if (strcmp(style, "bold") == 0) {
        printf("\033[1m%s\033[0m\n", text); // Жирный текст
    } else if (strcmp(style, "normal") == 0) {
        printf("%s\n", text); // Обычный текст
    } else if (strcmp(style, "small") == 0) {
        // Использование маленьких символов Unicode для имитации маленького текста
        printf("\033[0;2m%s\033[0m\n", text); // Устанавливаем тусклый текст
    }
}
void set_theme(const char *theme_color) {
    // Установка цвета текста
    if (strcmp(theme_color, "red") == 0) {
        printf("\033[0;31m"); // Красный текст
    } else if (strcmp(theme_color, "green") == 0) {
        printf("\033[0;32m"); // Зеленый текст
    } else if (strcmp(theme_color, "blue") == 0) {
        printf("\033[0;34m"); // Синий текст
    }
}
// Функция для сброса цвета текста к стандартному
void reset_color() {
    printf("\033[0m"); // Сбрасываем цвет текста к стандартному
}
void execute_command(char *cmd) {
   if (strncmp(cmd, "cd", 2) == 0) {
       // Команда "cd" обрабатывается отдельно
       char *arg = cmd + 2; // Ищем аргумент после "cd"
       while (*arg == ' ') arg++; // Пропускаем пробелы после "cd"


       if (*arg == '\0') {
           // Если нет аргумента, переходим в домашний каталог
           chdir(getenv("HOME"));
       } else {
           // Иначе пытаемся перейти в указанный каталог
           if (chdir(arg) != 0) {
               perror("cd");
           }
       }
       return;
   }else if (strncmp(cmd, "set_prompt", 10) == 0) {
        // Обработка команды "set_prompt" для установки пользовательского приглашения
        char *new_prompt = strtok(cmd + 11, "\n");
        if (new_prompt != NULL) {
            set_prompt(new_prompt); // Устанавливаем новое приглашение
            printf("Prompt set to: %s\n", prompt);
        } else {
            printf("Usage: set_prompt <new_prompt>\n");
        }
        return;
    }else if (strncmp(cmd, "set_text_style", 14) == 0) {
        char *style = strtok(cmd + 15, " ");
        char *text = strtok(NULL, "\n");
        if (style != NULL && text != NULL) {
            set_text_style(style, text);
        } else {
            printf("Usage: set_text_style <style> <text>\n");
        }
        return;
    }else if (strcmp(cmd, "reset_prompt") == 0) {
        reset_prompt();
        return;
    }
   else if (strncmp(cmd, "export", 6) == 0) {
        // Обработка команды "export" для установки переменной среды
        char *name = strtok(cmd + 7, " \t\n");
        char *value = strtok(NULL, "\n");
        if (name != NULL && value != NULL) {
            setenv(name, value, 1); // Установка переменной среды
        } else {
            printf("Usage: export <name> <value>\n");
        }
    }else if (strncmp(cmd, "set_all_style", 13) == 0) {
        char *style = strtok(cmd + 14, "\n");
        if (style != NULL) {
            set_all_style(style);
            printf("Стиль текста установлен на %s.\n", style);
        } else {
            printf("Usage: set_all_style <style>\n");
        }
        return;
    } else if (strncmp(cmd, "unset", 5) == 0) {
        // Обработка команды "unset" для удаления переменной среды
        char *name = strtok(cmd + 6, " \t\n");
        if (name != NULL) {
            unsetenv(name); // Удаление переменной среды
        } else {
            printf("Usage: unset <name>\n");
        }
} else if (strncmp(cmd, "set_theme", 9) == 0) {
        // Обработка команды "set_theme" для установки цветовой схемы
        char *theme_color = strtok(cmd + 10, " \t\n");
        if (theme_color != NULL) {
            set_theme(theme_color); // Установка цветовой схемы
            printf("Цветовая схема установлена на %s.\n", theme_color);
        } else {
            printf("Использование: set_theme <цвет>\n");
        }
        return;
}


   // Разделение ввода на команды по символу '|'
   char *token;
   char *commands[10]; // Предполагаем, что не более 10 команд
   int i = 0;
   token = strtok(cmd, "|");
   while (token != NULL) {
       commands[i++] = token;
       token = strtok(NULL, "|");
   }


   // Создание канала для каждой команды, кроме последней
   int num_commands = i;
   int pipes[num_commands - 1][2];
   for (int i = 0; i < num_commands - 1; i++) {
       if (pipe(pipes[i]) < 0) {
           perror("pipe");
           exit(EXIT_FAILURE);
       }
   }


   // Запуск каждой команды
   for (int i = 0; i < num_commands; i++) {
       pid_t pid = fork();
       if (pid == 0) {
           // Дочерний процесс
           // Проверка наличия перенаправления ввода/вывода
           char *input_file = NULL;
           char *output_file = NULL;
           char *command = commands[i];
           char *input_redirect = strchr(command, '<');
           char *output_redirect = strchr(command, '>');
           if (input_redirect) {
               *input_redirect = '\0'; // Заменяется символ '<' нулевым символом, чтобы разделить команду и имя файла.
               input_file = strtok(input_redirect + 1, " \t\n"); // Разбивается строка после символа '<' и извлекается имя файла.
               input_file = strtok(input_file, " \t\n"); // Если имя файла может быть отделено пробелами или табуляцией.
               int fd = open(input_file, O_RDONLY);
               if (fd < 0) {
                   perror("open");
                   exit(EXIT_FAILURE);
               }
               dup2(fd, STDIN_FILENO); // стандартный поток ввода! Ввод команды будет считываться не с клавиатуры, а из указанного файла.
               close(fd);
           }
           if (output_redirect) { // если найдено > 
               *output_redirect = '\0'; // Разделение команды и имени файла
               output_file = strtok(output_redirect + 1, " \t\n");
               output_file = strtok(output_file, " \t\n");
               int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666); // O_TRUNC удалит содержимое
               if (fd < 0) {
                   perror("open");
                   exit(EXIT_FAILURE);
               }
               dup2(fd, STDOUT_FILENO);
               close(fd);
           }

            // STDIN_FILENO = стандартный поток ввода 
           // Подключение каналов для конвейера
           if (i != 0) {
               // Если это не первая команда, подключаем ввод из предыдущего канала
               dup2(pipes[i - 1][0], STDIN_FILENO); // вывод прошлой команды -> ввод текущей команды
               close(pipes[i - 1][0]);
           }
           if (i != num_commands - 1) {
               // Если это не последняя команда, подключаем вывод в следующий канал
               dup2(pipes[i][1], STDOUT_FILENO); // вывод текущей команды -> ввод следующей команды
               close(pipes[i][1]);
           }


           // Закрытие оставшихся каналов
           for (int j = 0; j < num_commands - 1; j++) {
               close(pipes[j][0]);
               close(pipes[j][1]);
           }


           // Выполнение команды
           char *args[] = {"/bin/sh", "-c", command, NULL}; // -с означает что строка содержит команду 
           execv("/bin/sh", args);
           // Если execv() вернет управление, значит произошла ошибка
           perror("execv");
           exit(EXIT_FAILURE);
       } else if (pid < 0) {
           perror("fork");
           exit(EXIT_FAILURE);
       }
   }


   // Закрытие всех каналов в родительском процессе
   for (int i = 0; i < num_commands - 1; i++) {
       close(pipes[i][0]);
       close(pipes[i][1]);
   }


   // Ожидание завершения всех дочерних процессов
   for (int i = 0; i < num_commands; i++) {
       wait(NULL);
   }

    // fileno(stdout) - это файловый дескриптор, связанный с потоком вывода stdout; STDOUT_FILENO - это стандартный файловый дескриптор для потока вывода
   // Восстановление стандартного ввода и вывода в родительском процессе
   dup2(STDIN_FILENO, fileno(stdin));
   dup2(STDOUT_FILENO, fileno(stdout));
}


int main() {
   execute_command("clear");
   set_theme("blue");
   char *input;


   while ((input = readline(prompt)) != NULL) {
       if (strcmp(input, "exit") == 0) {
           free(input); //освобождается память
           break;
       }
        // Обрабатываем команды
        if (strcmp(input, "reset_colour") == 0) {
            reset_color();
            printf("Color reset to default.\n");
            free(input); // Освобождаем буфер ввода
            continue; // Пропускаем остальную обработку
        } 
       add_history(input); // Добавление введенной строки в историю


       printf("$ %s\n", input);
       execute_command(input);


       free(input);
   }


   return 0;
}
