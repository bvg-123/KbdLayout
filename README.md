# Проект KbdLayout
Комплекс утилит этого проекта (KbdLayoutMonitor, KbdColorer KbdRdpColorer) позволяет изменять цвет подсветки клавиш клавиатуры в зависимости от текущей раскладки (латинской, русской и т.д.) клавиатуры с поддержкой обычных, консольных- и RDP-окон.

Для чего нужна подсветка клавиатуры? Для понтов? Чтобы в темноте не промахиваться по клавишам?
А можно ли приспособить клавиатуру с подсветкой клавиш для чего-то более полезного?

До появления Windows 7 я искренне не понимал, зачем люди используют всякие пунтосвитчеры, вбивая целые слова или даже предложения в неправильной раскладке. Я всегда знал,
какая в данный момент у меня раскладка клавиатуры. Получалось примерно 90% времени - латинская раскладка клавиатуры, а 10% - кириллическая. Также стоит отметить, что до Windows 7
я использовал старую добрую клавиатуру BTC без идиотских windows-кнопок между Ctrl-ом и Alt-ом, поэтому легким движением большого и указательного пальцев левой руки
всегда легко и четко переключал раскладку на ту, которая была мне необходима в данный момент.

Внезапно много лет назад все в один момент изменилось. Затертая до дыр в клавишах старая клавиатура BTC и Windows XP остались на предыдущем месте работы. А на следующем новом месте работы была уже Windows 7.
Клавиатуру я мог заказать любую. Правда оказалось, что к тому времени производители клавиатур, все, как под копирку, производили убогие совершенно не пригодные для работы изделия.
Клавиши нажимались либо невнятно, либо туго с перекосами и скриптом. Человеческий Г-образный Enter куда-то исчез, вместо него стали лепить какую-то маленькую горизонтальную палочку,
по которой я стабильно ровно на 3 миллиметра промахивался мизинцем при "слепой" печати. И везде левые клавиши Ctrl и Alt разнесли на бОльшее расстояние, чтобы впендюрить между
ними совершенно бесполезную кнопку с Windows-флагом и максимально затруднить переключение раскладки клавиатуры. 

Последним гвоздем стало очередное "индусское" изобретение Microsoft
в области дружественных интерфейсов: в Windows 7 вместо одной текущей раскладки клавиатуры в каждом окне сделали свою собственную отдельную раскладку. Достаточно попереключаться
между несколькими окнами, повводить в них тексты в разных раскладках, чтобы совершенно потерять понимание того, какая буква (латинская или русская) будет в данный момент
напечатана при нажатии на конкретную нужную клавишу. С этого момента я напрочь перестал правильно угадывать, в какой раскладке я нахожусь и практически в 100% случаев вводил
слова и даже целые предложения не в той раскладке, каждый раз чертыхаясь и поминая недобрым словом дебилов из Microsoft, которые додумались привязать текущую раскладку не к
клавиатуре, а к окну. А как же индикатор текущей раскладки в системном трее? Чтобы посмотреть на этот мелкий убогий индикатор нужно оторвать глаза от текущего места ввода,
и найти этот индикатор, который находится вовсе не на одном месте, а путешествует влево и вправо по системному трею в зависимости от количества иконок в этом трее в текущий момент.
Понятное дело, что скорости все эти лишние манипуляции не добавляют. Пока их проделаешь, мысль, которую хотел записать, уже успеет убежать.

С этого момента я задался целью решить проблему, родившуюся из мирового заговора стараниями Microsoft и производителей клавиатур.

Начал с поисков более-менее приличной клавиатуры из тех, что были в наличии в магазинах. Тупо ходил по магазинам и нажимал на кнопки в поисках той клавиатуры, по основным используемым
кнопкам которой я не буду промахиваться. Процесс оказался долгим и трудным. В результате пришлось остановиться на клавиатуре Logitech G213. Клавиши в ней нажималсь относительно нормально и
по большей части основные были на своих местах. Enter был большим L-образным (найти нормальную клавиатуру с Г-образным Enter-ом мне, увы, не удалось). Дополнительным бонусом оказалось,
наличие в этой клавиатуре подсветки внутри клавиш, разбитой на 3 зоны, в каждой из которых можно было независимо изменять цвет подсветки. Для управления цветом подсветки Logitech предлагал
жутко громоздкое и крайне неудобное свое приложение Logitech Hub. Даже SDK к нему предлагал, чтобы можно было использовать возможности управления цветом подсветки из своих приложений.
Поигрался я с этим Logitech Hub-ом и с его API и приуныл - весь этот куцый, неудобный и убогий функционал, ориентированный, вероятно, на каких-то гипотетических сферических геймеров в вакууме, для моих целей оказался совершенно не жизнеспособным.

Однако, потенциальная возможность менять цвет подсветки клавиатуры в зависимости от ее текущей раскладки крепко засела у меня голове и не давала покоя. Для начала нужно было избавиться от
Logitech Hub-а с его "API" и научиться менять цвет подсветки легко и изящно без лишнего монструозного ПО. Сказано - сделано. Понятно, что Logitech Hub меняет цвет подсветки не по волшебству,
а посредством команд, отправляемых в клавиатуру по определенному протоколу через USB-порт, к которому клавиатура подключена. Тут возможны два варианта: либо попробовать найти документацию Logitech
с описанием используемого ими протокола для управления подсветкой клавиатуры, либо доустановить к сетевому сниферу Wireshark плагин USBPcap, при помощи которого поснифить USB-трафик и 
попробовать поймать в нем команду изменения цвета подсвтетки клавиатуры, отправляемую Logitech HUB-ом. Я использовал оба пути.

На просторах интернета нашлась небольшая кучка pdf-файлов от Logitech, в некоторых из которых, например, logitech_hidpp_2.0_specification_draft_2012-06-04.pdf удалось почерпнуть крупицы информации,
имеющие отношение к конкретным передаваемым по USB байтам команд управления теми или иными фичами их периферии, в т.ч. клавиатур. В частности, из этого документа можно узнать, что
для отправки информации в свои периферийные устройства Logitech использует протокол, который именует HID++2.0, и в котором предусмотрено два типа сообщений: длинные (по 20 байт) и короткие (по 7 байт).
Формат короткого сообщения согласно этому документу и с учетом специфики, полученной мною из анализа результатов снифинга USB-трафика моей клавиатуры Logitech G213, имеет вид:
```С
  //                          ┌─ Report ID. Определяет тип сообщений протокола: 0x11 - длинные по 20 байт, 0x10 - короткие по 7 байт.
  //                          │  ┌─ Device Index
  //                          │  │  ┌─ Feature Index
  //                          │  │  │ ┌─ Fctn/ASE Id - идентификатор функции
  //                          │  │  │ │┌─ Sw. Id - кастомный идентификатор, определяемый вызывающим ПО (без изменений вернется в ответе)
  //                          │  │  │ ││ 0x05-зона кнопок цифрового блока 
  //                          │  │  │ ││ 0x04-зона курсорных кнопок
  //                          │  │  │ ││ 0x03-зона правой части основного блока кнопок в районе Enter
  //                          │  │  │ ││ 0x02-зона средней части основного блока кнопок
  //                          │  │  │ ││ 0x01-зона левой части основного блока кнопок
  //                          │  │  │ ││ 0x00-все кнопки клавиатуры
  //                          │  │  │ ││ └┬─┘   ┌─ Красный (R), например, 0x8C
  //                          │  │  │ ││  │     │  ┌─ Зеленый (G), например, 0x61
  //                          │  │  │ ││  │     │  │  ┌─ Голубой (B), например, 0x1A
  //                         ̲ ̲│ ̲ ̲│ ̲ ̲│ ││ ̲ ̲│    ̲ ̲│ ̲ ̲│ ̲ ̲│
  { "vid_046d", "pid_c336", "11 FF 0C 3C 00 01 &R &G &B 02 00 00 00 00 00 00 00 00 00 00"}  // Logitech G213 Keyboard
```
О том, что означают коды vid_046d и pid_c336, я расскажу буквально абзацем ниже.

Таким образом, первая "творческая" половина пути по реализации утилитки для смены цвета подсветки клавиатуры пройдена. Осталось пройти оставшуюся техническую половину.
На ней меня поджидали неожиданные трудности. Надо заметить, что какие-то трудности есть всегда, а при использовании API Microsoft трудности усугубляются дополнительно
их совершенно отстойной и формальной документаций, читая которую постоянно ловишь себя на мысли, что эту документацию генерировала не иначе как какая-то нейронная сеть,
с убогой архитектурой, обученная на самых убогих комментариях из кода самых убогих из Microsoft-овских программистов.

После этого лирического отступления вернемся к кодам vid_046d и pid_c336. При организации передачи информации через USB возникает необходимость в идентификации конкретных, участвующих в обмене устройств,
поскольку было бы опрометчиво пытаться отправить команду смены цвета подсветки не клавиатуре, а мыши, или второй клавиатуре, не обладающей функцией подсветки клавиш. Код vid_046d,
как раз и означает, что мы имеем дело именно с периферийным устройством, произведенным Logitech. А код pid_c336 означает, что этим устройством является клавиатура G213. Таким образом,
прежде, чем начинать отправлять в эту клавиатуру те или иные команды, необходимо эту клавиатуру найти среди всего многообразия периферии, которая может быть в данный момент подключена
к USB-портам компьютера. Делается это путем банального перебора всех подключенных USB-устройств, получением от них идентификатора вендора/производителя (vid_046d) и кода конкретного устройства (pid_c336),
произведенного этим вендором. Такой перебор можно произвести, пользуясь следующими функциями Windows API:
```
  Начало перебора устройств SetupDiGetClassDevs
  Цикл по SetupDiEnumDeviceInterfaces
    Получение внутри цикла информации об очередном устройстве SetupDiGetDeviceInterfaceDetail
    Идентификация в полученной строке DevicePath (путь к "виртуальному файлу" устройства) нужных нам кодов vid_* и pid_*
    Проверка возможности открыть "виртуальный файл" устройства для записи (т.е. для отправки в него команд)
  Завершение перебора устройств SetupDiDestroyDeviceInfoList
```
Самым интересным в этом цикле является именно проверка возможности открыть "виртуальный файл" устройства для записи. Дело в том, что несмотря на то, что к компьютеру подключена только одна клавиатура
Logitech G213, поиск всех DevicePath, содержащих коды vid_046d и pid_c336, даст нам несколько таких DevicePath, отличающихся некоторыми нюансами (дополнительными символами).

Дело в том, что любое,
подключенное к USB устройство может иметь несколько точек для взаимодействия с ним. Здесь можно провести аналогию с сетевыми портами компьютера, когда по одному порту (например, 80) запросы принимает веб-сервер,
через другой порт (например, 139) происходит вызов удаленных процедур RPC и т.д. Так вот, для каждой точки (endpoint), через которую можно взаимодействовать с USB-устройством, наш цикл по SetupDiEnumDeviceInterfaces
даст свой собственный отдельный DevicePath. Через один из этих endpoint-ов можно только читать коды клавиш, нажимаемых на нашей клавиатуре, через другой - отдавать команды смены цвета подcветки и т.д.
Попытка отправить команду смены цвета подсветки в endpoint, предназначенный для чтения кодов клавиш, ни к чему хорошему не приведет. Windows просто не позволит это сделать, вернув скромную ошибку "Access denied"
при попытке открыть "виртуальный файл" устройства по соответствующему такому endpoint-у пути, предоставив вам отличную возможность бесконечно долго гадать о возможных причинах (достаточно ли прав у аккаунта, от которого
выполняется код, может быть нужно отключить UAC или запуститься в режиме "Run As Administrator" и т.д.). А причина на самом деле проста. Таким своеобразным способом microsoft-овцы "защищают" пользователя
от "программ шпионов", которые не дай бог смогут прочитать что-то лишнее из нажимаемых пользователем клавиш, например, секретный пароль в интернет-банк. Если честно, то на этом месте я промучался довольно долго
в поисках возможных причин интерпретации этого "Access denied" и путей его обхода или обмана. В конце концов для полноты картины я решил собрать все "виртуальные файлы", в путях которых встречались коды моей клавиатуры,
и для эксперимента попробовать поочередно пооткрывать их API-шной функцией CreateFile с разными наборами параметров (на чтение, на запись, и на чтение и на запись, с разделяемым доступом, с монопольным доступом и т.д.
во всех возможных комбинациях). В результате нашелся один из множества DevicePath (с требуемыми vid-ом и pid-ом), успешно открываемый отмычкой из параметров:
  GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE и OPEN_EXISTING.

Если вы думаете, что тут можно было бы взохнуть с облегчением, то вы плохо знаете ребят из Microsoft. Простые и прямые пути к достижению цели - это не про них. Их стезя - приносить в мир боль и страдания.

Ну вот мы открыли "виртуальный файл" нашей клавиатуры. Теперь надо бы в него записать кучку байт с командой смены цвета. Но в каком виде их писать? И какими API-шными функциями? WriteFile? HidD_SetOutputReport?
Что-нибудь еще более забористое? И в каком виде кидать нашу кучку байтов? Как есть? Или с каким-то специальным образом подготовленным заголовком (HidP_SetData, HidP_SetScaledUsageValue, HidP_SetUsageValueArray)?
Изучение документации Microsoft по этим функциями ясности не прибавило, а тумана нагнало изрядно. Однако, всегда остается старый добрый "Маэстро эксперимент". Попробовал я покидать разные байтики в хендл открытого "виртуального файла"
этими API-шными функциями в разных сочетаниях и поснифить результаты Wireshark-ом. Понятное дело, что в 99 случаях из 100 я получал одни и те же замечательные в своей лаконичности ошибки типа ERROR_INVALID_FUNCTION, ERROR_ACCESS_DENIED,
ERROR_INVALID_PARAMETER. Какое именно значение и какого параметра не нравится? Какая именно функция не работает у инвалидов, слепивших эту операционку? Ни логов, ничего...

И тут я вспомнил про длинные и короткие сообщения. А ведь не только MS-овцы, но и Logitech-овцы могли подложить подлянку. Так и есть. Под сообщения разной длины у них заточены разные endpoint-ы.
Не вопрос. Добавляем в наш цикл дополнительную проверочку на размер буфера, report в котором готово принимать USB-устройство через данный endpoint. И вот очередной подходящий DevicePath, отличный от ранее опробованных.
Записываем в него байтики через HidD_SetOutputReport и видим ровно те же самые записанные байтики в выловленном Wireshark-ом из USB-шины пакете. А цвет подсветки клавиатуры неожиданно становится нежно фиолетовым
(в крапинку).

Ну что ж. Одна часть задачи решена. Исходный код получившейся у меня утилиты для смены цвета подсветки клавиатуры в зависимости от переданного ей в командной строке кода раскладки лежит в подкаталоге KbdLayoutIndicators\KbdColorer.

Как же теперь определить текущую раскладку клавиатуры? "Энциклопелия юных сурков" (MSDN Library от 1999 года, когда она еще распространялась на DVD, имела нормальный индекс с поиском и не была безнадежно испорчена
автогенераторами help-ов и прочим лукавым сайтостроительством) говорит, что раскладка клавиатуры в Windows относится вовсе не к клавиатуре, а к потоку (thread-у) оконной процедуры каждого из имеющихся в системе окон,
коих там превеликое множество. И самое страшное, что в каждом таком GUI-потоке может быть своя собственная раскладка клавиатуры! Кошмар, документированный в теории 1999 года и воплощенный в безальтернативном варианте
реализации в Windows 7! 

Но это - лирика. В сухом же остатке есть несколько возможностей:

1. Сделать клавиатурный хук (keyboard hook) в виде dll-ки, внедрить ее во все запущенные и запускаемые процессы и ловить этим хуком сочетание клавиш переключения раскладки. Вариант - так себе, поскольку переключить раскладку
клавиатуры можно и мышкой через индикатор текущей раскладки в системном трее. А еще есть RDP-окна от mstsc.exe, которые автоматически сносят любые переустановленные хуки, наверное, из благих соображений пресловутой
"безопасности" пользователя.

2. Сделать CBT-хук через SetWindowsHookEx с параметром WH_SHELL. Таким хуком можно было бы теоретически перехватить смену раскладки мышкой, если бы документация MS скромно не умолчала об одном важном факте,
что эти хуки работать в обычном приложении не заставишь. Приложение должно быть shell-ом, а-ля Проводник (Windows Explorer). А написать собственный шелл - это не два байта переслать. Короче, тоже так себе вариант,
особенно, с учетом частого использования RDP через mstsc, который, как ивестно, с завидной периодичностью перешибает любые установленные хуки на свои собственные.

3. С завидной периодичностью... Получается нам нужен таймер или еще лучше - небольшой цикл, который будет периодически копировать изображение с индикатора раскладки клавиатуры в системном трее и распознавать скопированное изображение. Я некоторое время назад развлекался прогулкамии по памяти bitmap-а из managed-кода C# (репозиторий [RusLat](https://github.com/bvg-123/RusLat)) и различными относительно тривиальными алгоритмами распознавания содержимого этого bitmap-а. Впрочем можно использовать артиллерию и более крупного калибра. Можно скормить картинку с изображением индикатора раскладки клавиатуры предобученной нейронной сети, например, построенной на архитектуре ResNet. И эта нейросеть будет кластеризовать изображение индикатора на сегменты и относить в одну из групп, соответствующих латинской или кириллической раскладке.

Классный вариант. Из чистого любопытства давно собирался посмотреть ML-реализацию имени MS и сравнить с Tensorflow/Python-вским подходом. Реализовал я такую нейросетку в С#-проекте RusLat2.csproj. Получилось красиво и с обучением под конкретные шрифты, цвета и темы. Все бы ничего, только инференс этой нейронки поджирает CPU. Хорошо так поджирает.
Если раскладка не меняется, все равно приходится в цикле распознавать картинку на индикаторе, чтобы убедиться, что она не меняется. А срезать угол через хуки - увы не получится. Кстати, что же в этом варианте с RDP?
А плохо все с RDP. При переключении в RDP-окно, индикатор раскладки в системном трее просто исчезает. Это значит, что нужно проверять, а не RDP-ли окно у нас наверху в foreground-е? Если RDP, то снимать с него копию изображения в 
виде картинки и скармливать нейронке. Только нейронка нужна позабористей. А как быть, если изображение системного трея в RDP-окне уползает вниз за пределы окна, как это часто бывает, когда приходится работать с несколькими
одновременно отображаемыми на экране RDP-окнами уменьшенного размера. Можно было бы подменить device context в оконной процедуре RDP-окна и заставить его нарисовать свое содержимое на имитируемом в памяти "виртуальном" экране
любого нужного размера. Попробовал. В лоб простыми средствами заставить RDP-окно нарисовать картинку удаленного рабочего стола в его натуральную величину вне зависимости от размера самого RDP-окна невозможно. Обходные
обманные варианты с заменой на лету байтов в win32k.sys для перехвата вызовов BeginPaint и EndPaint (типа такого http://www.fengyuan.com/article/wmprint.html), работающие для обычных окон, для RDP-окна не прокатили
из-за особенностей RDP-протокола и той самой "безопасности".

4. Поскольку срезать угол никак не получилось, придется действовать относительно честно. Будем периодически в цикле просыпаться, скажем раз в 100 мс и проверять текущее верхнее окно. Если оно не меняется, то продолжать спать до
следующей итерации цикла. А если верхнее окно сменилось, то находим создавший его поток (точнее поток, создавший очередь сообщений этого окна) и через API-шную функцию GetKeyboardLayout получаем раскладку клавиатуры в
этом потоке, ловим момент, когда она сменится и вызываем нашу утилитку для смены цвета подсветки клавиатуры. Это практически железобетонный вариант, который традиционно не прокатывает с RDP-окном, а также, как это
ни странно, с консольными окнами, в которых я провожу в Far-е бОльшую часть времени, когда не сижу в рабочем RDP. С RDP правда в этом варианте можно и побороться. Достаточно запустить на удаленном хосте мониторинг верхнего
foreground-вого окна с определением смены раскладки в его потоке и вызовом вместо утилиты смены цвета клавиатуры другой утилиты, которая будет отправлять с RDP-хоста на RDP-клиент по виртуальному RDP-каналу сигнал с новой раскладкой клавиатуры RDP-хоста. RDP-протокол устроен таким образом, что через него теоретически можно передать что угодно кастомное (см. https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-server-application и https://learn.microsoft.com/en-us/windows/win32/termserv/virtual-channel-client-registration).

Так с RDP-разобрались. Осталось выяснить, почему не получается определить раскладку клавиатуры консольного окна. Орешек с консольным окном оказался крепче, чем может показаться на первый взгляд. Дело в том, что найти "GUI-поток", связанный с данным консольным окном не так-то просто. Консольными окнами в современных версиях Windows заведует отдельный
процесс conhost.exe. Он, кстати, не один - их может быть много. И потоков в каждом из них тоже несколько. Как определить, какой из них нам нужен? Поэксперименировал я с этими консольными процессами и потоками.
Даже матчасть нашел - исходники этого самого conhost.exe https://github.com/microsoft/terminal. Бинго! В этих исходниках видно, как ребята из MS обслуживают очередь сообщений и как устроена оконная процедура.
Таким образом, побороть консоль нам поможет GetGUIThreadInfo и сканированое всех потоков системы, относящихся к процессу conhost.exe, родителем которого является процесс, связь которого
с консольным окном можно определить через GetWindowThreadProcessId. Результат не заставил себя ждать и раскладка клавиатуры в консольных окнах стала определяться на раз (см. модуль conhost.cpp в проекте KbdLayoutMonitor).

Вариант 4 я в итоге и реализовал. Он одинаково хорошо работает и с обычными окнами, и с консольными и даже с RDP-окнами при условии запуска на удаленном RDP-хосте собственного KbdLayoutMonitor-а.
Исходники прилагаются. Кому надо - пользуйтесь на здоровье, только с обязательной ссылкой на меня, как автора (Богдан Гарбузов, b.garbuzov@hotmail.com)!


P.S. Особых наворотов C++ я не использовал, только обычный C. Поскольку в настоящее время по работе пишу в основном на C# и Python-е, а C++ активно не использовал года эдак с 2001, то код устроен предельно просто и примитивно,
как "извилина в голове у каратиста".


P.P.S. После того, как я стал использовать KbdLayoutMonitor, KbdColorer и KbdRdpColorer я с удовлетворением отметил, что полностью перестал путаться с текущей раскладкой клавиатуры. Теперь у меня в латинской
раскладке клавиатура светится приятным желто-зеленым цветом, а в русской раскладке - нежным светло-бордовым, как бы я не переключался между локальной Visual Studio, удаленной по RDP Visual Studio и запущенными
локально и по RDP Far-ами и прочими командными строками.
