# PRÁCTICA 1 : COMUNICACIÓN ENTRE PROCESOS 

## Descripción general

Este programa implementa un sistema de busqueda eficiente sobre un conjunto de datos en formato csv, utilizando 
el dataset **books processed dataset** obtenido de Kaggle. Permite al usuario realizar consultas rápidas
mediante un sistema de indexación basado en una **Tabla Hash**, comunicando dos procesos no emparentados a tráves de **Tuberías Nombradas (FIFO)**.

## Campos del dataset

| Campo                   | Tipo de dato | Descripción |
|--------------------------|--------------|--------------|
| `title`                  | String       | Título del libro. |
| `author_name`            | String       | Nombre del autor o autores. |
| `image_url`              | String / URL | Enlace a la imagen de la portada del libro. |
| `num_pages`              | Entero       | Número total de páginas del libro. |
| `average_rating`         | Flotante     | Calificación promedio otorgada por los usuarios. |
| `text_review_count`      | Entero       | Número total de reseñas escritas por los usuarios. |
| `description`            | String       | Sinopsis o resumen del contenido del libro. |
| `5_star_rating_counts`   | Entero       | Cantidad de calificaciones de 5 estrellas. |
| `4_star_rating_counts`   | Entero       | Cantidad de calificaciones de 4 estrellas. |
| `3_star_rating_counts`   | Entero       | Cantidad de calificaciones de 3 estrellas. |
| `2_star_rating_counts`   | Entero       | Cantidad de calificaciones de 2 estrellas. |
| `1_star_rating_counts`   | Entero       | Cantidad de calificaciones de 1 estrella. |
| `total_rating_counts`    | Entero       | Total de calificaciones. |
| `genres`                 | String       | Géneros literarios asociados al libro. |

## Criterios de Búsqueda Implementados



