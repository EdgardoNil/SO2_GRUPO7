docker run --name mysql-db -d -p 3306:3306 -e MYSQL_ROOT_PASSWORD=SO2P1$ mysql
-- Nombre de la base de datos: Memoria
CREATE DATABASE Memoria;

USE Memoria;

-- Creación de la tabla de solicitudes
CREATE TABLE IF NOT EXISTS solicitud (
    id INT AUTO_INCREMENT PRIMARY KEY,
    pid INT,
    proceso VARCHAR(256),
    llamada VARCHAR(256),
    tamano float,
    fecha_hora VARCHAR(20),
    porcentaje float
);

