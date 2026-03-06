FROM gcc:latest

WORKDIR /usr/src/app

COPY . .

RUN gcc main.c -o server

EXPOSE 1080

CMD ["./server", "1080"]