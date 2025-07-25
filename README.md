# audioFileAnalyzer

# Описание
Приложение для анализа аудиофайлов в формате WAV с визуализацией

# Функциональность
1. Загрузка и воспроизведение аудио:
    - Поддержка формата WAV
    - Воспроизведение с управлением громкостью
    - Ползунок перемотки
    - Кнопки управления: Play/Pause/Stop
2. Отображение метаданных аудиофайла:
    - Длительность
    - Частота дискретизации
    - Битрейт
    - Число каналов
    - Битность
3. Визуализация:
    - Осциллограмма:
        - Отображение осциллограммы
        - Масштабирование колесом мыши
        - Маркер текущей позиции воспроизведения
    - Спектрограмма:
        - Отображение спектрограммы
    - Спектр
        - Отображение амплитудно-частнотной характеристики
        - Логарифмическая шкала частот (20 Гц - 20 кГц)
        - Масштабирование при помощи выделения участка левой кнопкой мыши и прокрутка колесом мыши
# Кодстайл
camelCase для переменных и методов, PascalCase для классов

# Инструкция по сборке
1. Скачиваем Qt и Qt Creator с официального сайта: https://www.qt.io/download-open-source
2. В Qt Creator выбираем: Файл -> New Project -> Импортировать проект -> Клонирование Git
3. В строке "Хранилище" вводим https://github.com/GZhurkin/audioFileAnalyzer.git, в строке "Ветка" вводим "main"
4. Далее выбираем набор инструментов и завершаем найстройку проекта
5. Запускаем проект (Ctrl + R)