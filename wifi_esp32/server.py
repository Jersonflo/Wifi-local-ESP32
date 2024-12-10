from flask import Flask, request, render_template_string, redirect, url_for, Response
import csv
import os
import re  # Usado para validaciones con expresiones regulares
from asistencia_control import delete_registros, download_registros

app = Flask(__name__)

# Ruta y nombre del archivo CSV
CSV_FILE = 'asistencia.csv'

# Usuario y contraseña (puedes personalizarlo)
USERNAME = 'admin'
PASSWORD = 'admin123'

# Página HTML con formulario
html = '''
<!DOCTYPE html>
<html>
  <head>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
        margin-top: 50px;
        background-color: #f4f4f4;
      }
      form {
        display: inline-block;
        padding: 20px;
        background: white;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      }
      input, select {
        margin: 10px 0;
        padding: 9px;
        width: 90%;
        max-width: 300px;
      }
      input[type="submit"] {
        background: #007bff;
        color: white;
        border: none;
        cursor: pointer;
      }
    </style>
  </head>
  <body>
    <h1>Registro de Asistencia</h1>
    <form action='/submit' method='POST'>
      <input type='text' name='nombre' placeholder='Nombre y Apellido' pattern='[A-Za-z]+\\s[A-Za-z]+' title='Debe ingresar al menos nombre y apellido' required><br>
      <input type='text' name='celular' placeholder='Celular (10 dígitos)' pattern='\\d{10}' title='El celular debe tener 10 dígitos y no debe contener espacios, comas o puntos' required><br>
      <input type='email' name='correo' placeholder='Correo electrónico' required><br>
      <select name='proyecto' required>
        <option value="">Selecciona un proyecto</option>
        <option value="Videojuego">Videojuego</option>
        <option value="Sistema de IA">Sistema de IA</option>
      </select><br>
      <input type='number' name='horas' placeholder='Horas asistidas' min='1' max='24' required><br>
      <input type='hidden' name='fecha' id='fecha'>
      <input type='submit' value='Registrar'>
    </form>
    <script>
      document.getElementById('fecha').value = new Date().toISOString().split('T')[0];
    </script>
    <p><a href="/download">Descargar archivo CSV</a></p>
    <p><a href="/delete">Eliminar todos los registros</a></p>
  </body>
</html>
'''

# Ruta principal: muestra el formulario
@app.route('/')
def formulario():
    return render_template_string(html)

# Ruta para manejar el formulario
@app.route('/submit', methods=['POST'])
def submit():
    nombre = request.form.get('nombre').strip()
    celular = request.form.get('celular').strip()
    correo = request.form.get('correo').strip()
    proyecto = request.form.get('proyecto').strip()
    horas = request.form.get('horas').strip()
    fecha = request.form.get('fecha').strip()

    # Validaciones del servidor
    errors = []

    # Validación del nombre: debe tener al menos dos palabras (nombre y apellido)
    if not re.match(r'^[A-Za-z]+\s[A-Za-z]+$', nombre):
        errors.append('El nombre debe tener al menos dos palabras (nombre y apellido).')

    # Validación del celular: solo debe contener 10 dígitos
    if not re.match(r'^\d{10}$', celular):
        errors.append('El celular debe contener exactamente 10 dígitos, sin puntos ni comas.')

    # Validación del correo electrónico
    if not re.match(r'^[\w\.-]+@[\w\.-]+\.\w+$', correo):
        errors.append('El correo electrónico no tiene un formato válido.')

    # Validación del proyecto: solo debe ser una de las dos opciones
    if proyecto not in ['Videojuego', 'Sistema de IA']:
        errors.append('El proyecto debe ser "Videojuego" o "Sistema de IA".')

    # Validación de horas asistidas: deben estar entre 1 y 24
    try:
        horas_int = int(horas)
        if horas_int < 1 or horas_int > 24:
            errors.append('Las horas asistidas deben estar entre 1 y 24.')
    except ValueError:
        errors.append('Las horas asistidas deben ser un número entero válido.')

    # Validar si ya existe un registro con el mismo correo y la misma fecha
    if os.path.exists(CSV_FILE):
        with open(CSV_FILE, 'r') as file:
            reader = csv.reader(file)
            next(reader)  # Saltar la cabecera
            for row in reader:
                correo_existente = row[2]  # El correo está en la columna 2
                fecha_existente = row[5]  # La fecha está en la columna 5
                if correo_existente == correo and fecha_existente == fecha:
                    errors.append(f'Ya existe un registro para el correo {correo} en la fecha {fecha}.')

    # Si hay errores, se muestran al usuario
    if errors:
        return f'<h1>Errores de validación:</h1><ul>{"".join([f"<li>{error}</li>" for error in errors])}</ul><a href="/">Regresar</a>'

    # Si no existen errores, se guarda la información en el CSV
    if not os.path.exists(CSV_FILE):
        with open(CSV_FILE, 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(['Nombre', 'Celular', 'Correo', 'Proyecto', 'Horas', 'Fecha'])

    with open(CSV_FILE, 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([nombre, celular, correo, proyecto, horas, fecha])

    return '<h1>Asistencia registrada correctamente!</h1><a href="/">Regresar</a>'

# Ruta para descargar el archivo CSV (protegida por autenticación)
@app.route('/download')
def download():
    auth = request.authorization
    if not auth or auth.username != USERNAME or auth.password != PASSWORD:
        return Response('Acceso denegado', 401, {'WWW-Authenticate': 'Basic realm="Login requerido"'})
    return download_registros()

# Ruta para eliminar los registros (protegida por autenticación)
@app.route('/delete')
def delete():
    auth = request.authorization
    if not auth or auth.username != USERNAME or auth.password != PASSWORD:
        return Response('Acceso denegado', 401, {'WWW-Authenticate': 'Basic realm="Login requerido"'})
    return delete_registros()

# Iniciar el servidor Flask
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
