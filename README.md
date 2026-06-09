# Truten - система записи в спортивные залы

Десктопное приложение (Qt/QML) + C++ сервер (Crow) + PostgreSQL.

---

## Требования

- Ubuntu 22.04+
- Docker и Docker Compose
- Git

---

## 1. С чего начать

1. **Клонируйте репозиторий**:
   ```bash
   git clone https://github.com/ilovemoloko/Truten.git
   cd Truten
   ```

2. **Подготовьте окружение**:
   ```bash
   ./install.sh
   ```
   *Скрипт создаст файл `.env` и соберет Docker-образ сервера.*

3. **Настройте `.env`**:
   Откройте созданный файл `.env` и укажите свои данные для SMTP (почты), чтобы работали уведомления:

4. **Запустите сервер**:
   ```bash
   ./run_server.sh
   ```
   
---

## 2. Запуск клиента

```bash
./run_client.sh
```
*Скрипт установит необходимые Qt6-зависимости в систему, соберет клиент и запустит его.*

---


## Структура проекта

```
Truten/
├── server/          
│   ├── Dockerfile  
│   ├── src/         
│   └── include/     
├── client/          
│   ├── src/         
│   ├── qml/         
│   └── build/       
├── install.sh       
├── run_server.sh    
├── run_client.sh   
└── docker-compose.yml 
```
