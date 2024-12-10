import os
from flask import send_file

# Ruta y nombre del archivo CSV
CSV_FILE = 'asistencia.csv'

def delete_registros():
    """Elimina todos los registros en el archivo CSV."""
    if os.path.exists(CSV_FILE):
        os.remove(CSV_FILE)
        return '<h1>Registros eliminados correctamente.</h1><a href="/">Regresar</a>'
    else:
        return '<h1>No hay registros para eliminar.</h1><a href="/">Regresar</a>'

def download_registros():
    """Descarga el archivo CSV con los registros."""
    if not os.path.exists(CSV_FILE):
        return 'Archivo no encontrado'
    return send_file(CSV_FILE, as_attachment=True)
