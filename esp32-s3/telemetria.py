import socket
import ssl
import time
import paho.mqtt.client as mqtt
import sqlite3
import os

NAME = "telemetria_S.db"
HOST = '127.0.0.1'
PORT_HTTP = 8080
SSL = False

BROKER = '127.0.0.1'
PORT_MQTT = 1883

MAXTAM = 50
MAXTIE =5.0

db = sqlite3.connect(NAME, check_same_thread=False)



#mqtt



def db_begin():
    cursor = db.cursor()
    cursor.execute("""CREATE TABLE IF NOT EXISTS cola(id INTEGER PRIMARY KEY AUTOINCREMENT, payload BLOB)""")
    db.commit()

db_begin()


def on_men(cliente, user, men):
    try:
        pay = men.payload
        payL = pay.strip()
        if not payL.startswith(b'{') or not payL.endswith(b'}'):
            return
        cursor = db.cursor()
        cursor.execute("INSERT INTO cola (payload) VALUES (?)", (payL,))
        db.commit()
    except Exception as e:
        print(f"[!] Error al insertar en el disco: {e}")

cliente = mqtt.Client()
cliente.on_message = on_men
cliente.connect(BROKER, PORT_MQTT)
cliente.subscribe("robot/telemetria")
cliente.loop_start()



#https



def coneccion():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(10)
    if SSL:
        context = ssl.create_default_context()
        sock_s = context.wrap_socket(s, server_hostname=HOST)
        sock_s.connect((HOST, PORT_HTTP))
        return sock_s
    else:
        s.connect((HOST, PORT_HTTP))
        return s
while True:
    try:
        ultE = time.time()
        print("conectando sl https")
        sock = coneccion()
        while True:
            cursor = db.cursor()
            cursor.execute("SELECT id, payload FROM cola ORDER BY id LIMIT ?", (MAXTAM,))
            fil = cursor.fetchall()

            tiempo = time.time() - ultE

            lleno = len(fil) >= MAXTAM
            tiemp = (tiempo >= MAXTIE) and (len(fil) > 0)
            if lleno or tiemp:
                idsB = [f[0] for f in fil]
                pay = [f[1] for f in fil]

                json = b'[' + b','.join(pay)+ b']'

                tam = len(json)
                enc = (f"POST / HTTP/1.1\r\n"
                    f"Host: {HOST}\r\n"
                    f"Content-Type: application/json\r\n"
                    f"Content-Length: {tam}\r\n"
                    f"Connection: keep-alive\r\n"
                    f"\r\n")

                enc_b = enc.encode('utf-8')

                telemetria = enc_b + json

                print("conectandose")

                sock.sendall(telemetria)
                resp = sock.recv(4096)
                resp_s = resp.decode('utf-8', errors='ignore')
                if "200 OK" in resp_s or "201 Created" in resp_s:
                    ids_s = ','.join(map(str, idsB))
                    cursor.execute(f"DELETE FROM cola WHERE id IN ({ids_s})")
                    db.commit()
                    print(f"Enviados {len(idsB)} registros de telemetría")
                else:
                    print(f"[!] Error en la respuesta del servidor https: {resp_s}")
                    raise ConnectionResetError("se cerro la conexion https")


                ultE = time.time()
                time.sleep(0.1)
            else:
                time.sleep(0.1)
    except Exception as e:
        print(f"\n[!] Error de Conexión https: {e}, reintentando")
        time.sleep(3)
