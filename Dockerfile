FROM alpine:latest
WORKDIR /app
COPY ./build/bin/app .
CMD ["./app"]
