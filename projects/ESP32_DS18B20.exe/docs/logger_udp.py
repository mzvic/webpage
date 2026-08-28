import socket
import struct
import datetime
import csv
 
UDP_IP = "0.0.0.0" 
UDP_PORT = 12345
 
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
 
print(f"Escuchando datos UDP en el puerto {UDP_PORT}...")
 
tiempo_inicio = datetime.datetime.now()
str_inicio = tiempo_inicio.strftime("%Y%m%d_%H%M%S")
nombre_archivo = f"registro_temperaturas_{str_inicio}.csv"
 
print(f"Los datos se guardarán en el nuevo archivo: {nombre_archivo}\n")
 
offset_ms = None
with open(nombre_archivo, mode='w', newline='') as archivo_csv:
    escritor_csv = csv.writer(archivo_csv)
    escritor_csv.writerow(["Hora Local", "Tiempo Relativo (ms)", "T. Ambiente (C)", "T. Antena (C)"])
 
    try:
        while True:
            data, addr = sock.recvfrom(1024)
 
            ahora = datetime.datetime.now()
            hora_formateada = ahora.strftime("%H:%M:%S.%f")[:-3]
 
            timestamp_esp32_crudo, temp_amb, temp_ant = struct.unpack('<Iff', data)
 
            if offset_ms is None:
                offset_ms = timestamp_esp32_crudo
                print(f"Sincronizando tiempo... Offset inicial guardado: {offset_ms} ms")
 
            tiempo_relativo = timestamp_esp32_crudo - offset_ms
 
            escritor_csv.writerow([hora_formateada, tiempo_relativo, f"{temp_amb:.2f}", f"{temp_ant:.2f}"])
            print(f"[{hora_formateada}] t= {tiempo_relativo} ms | Ambiente: {temp_amb:.2f} °C | Antena: {temp_ant:.2f} °C")
 
    except KeyboardInterrupt:
        print(f"\nCaptura finalizada de forma segura.")
        print(f"El archivo {nombre_archivo} se ha cerrado y guardado correctamente.")
        sock.close()