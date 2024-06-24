from flask import Flask, jsonify
from flask_cors import CORS
import mysql.connector
import os
from dotenv import load_dotenv

app = Flask(__name__)
CORS(app) 

load_dotenv()

# Configuración de la conexión a la base de datos
db_config = {
    'host': os.getenv('DB_HOST'),
    'user': os.getenv('DB_USER'),
    'password': os.getenv('DB_PASSWORD'),
    'database': os.getenv('DB_NAME')
}
# Función para obtener los datos de la base de datos
def obtener_datos_grafica():
    try:
       
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        # Consulta SQL para obtener los datos
        query = '''SELECT
                    pid,
                    MIN(proceso) AS proceso,
                    SUM(tamaño_mmap) - SUM(tamaño_munmap) AS memoria_usada
                FROM (
                    SELECT
                        pid,
                        proceso,
                        CASE WHEN llamada = 'mmap2' THEN tamano ELSE 0 END AS tamaño_mmap,
                        CASE WHEN llamada = 'munmap' THEN tamano ELSE 0 END AS tamaño_munmap
                    FROM solicitud
                ) AS subconsulta
                GROUP BY pid, proceso
                ORDER BY memoria_usada DESC'''

        cursor.execute(query)

        rows = cursor.fetchall()

        data = []
        totalOtros = 0
        totalUso = 0
        memoriaTotal = 8589934592 

        for i, row in enumerate(rows):
            data.append({
                'pid': row[0],
                'proceso': row[1],
                'memoria_usada': row[2] if row[2] > 0 else 0,
                'porcentaje': round(row[2] / memoriaTotal * 100, 2)
            })
            totalOtros += int(row[2]) if int(row[2]) > 0 and i > 10 else 0
            totalUso += int(row[2]) if int(row[2]) > 0 else 0

        data.append({
            'pid': '',
            'proceso': 'Otros',
            'memoria_usada': totalOtros,
            'porcentaje': round(totalOtros / memoriaTotal * 100, 2)
        })

        data.append({
            'pid': '',
            'proceso': 'Libre',
            'memoria_usada': memoriaTotal - totalUso,
            'porcentaje': round((memoriaTotal - totalUso) / memoriaTotal * 100, 2)
        })

        # Cerrar conexión y retornar los datos
        cursor.close()
        conn.close()
        return data
    except Exception as e:
        return str(e)

# Función para obtener los datos de la base de datos
def obtener_datos():
    try:
        
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        query = "SELECT * FROM solicitud"

        cursor.execute(query)

        rows = cursor.fetchall()

        # Convertir los datos a un formato JSON
        data = []
        for row in rows:
            data.append({
                'pid': row[1],
                'proceso': row[2],
                'llamada': row[3],
                'tamano': row[4],
                'fecha_hora': row[5],  
                'porcentaje': row[6]
            })

        # Cerrar conexión y retornar los datos
        cursor.close()
        conn.close()
        return data
    except Exception as e:
        return str(e)

# Función para obtener los últimos 100 datos de la base de datos
def obtener_ultimos_datos():
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        query = "SELECT * FROM (SELECT * FROM solicitud ORDER BY id DESC LIMIT 100) AS ultimos ORDER BY id ASC"

        cursor.execute(query)


        rows = cursor.fetchall()

        # Convertir los datos a un formato JSON
        data = []
        for row in rows:
            data.append({
                'pid': row[1],
                'proceso': row[2],
                'llamada': row[3],
                'tamano': row[4],
                'fecha_hora': row[5], 
                'porcentaje': row[6]
            })

        # Cerrar conexión y retornar los datos
        cursor.close()
        conn.close()
        return data
    except Exception as e:
        return str(e)


# Ruta para obtener los últimos 15 datos
@app.route('/ultimadata', methods=['GET'])
def ultimos_datos():
    return jsonify(obtener_ultimos_datos())


# Ruta para obtener los datos
@app.route('/datos', methods=['GET'])
def datos():
    return jsonify(obtener_datos())


# Ruta para obtener los datos
@app.route('/datagrafica', methods=['GET'])
def datos_grafica():
    return jsonify(obtener_datos_grafica())

if __name__ == '__main__':
    app.run(debug=True)
