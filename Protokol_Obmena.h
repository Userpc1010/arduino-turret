


String sp_startMarker;           // Переменная, содержащая маркер начала пакета
String sp_stopMarker;            // Переменная, содержащая маркер конца пакета
String sp_dataString;            // Здесь будут храниться принимаемые данные
int sp_startMarkerStatus;  // Флаг состояния маркера начала пакета
int sp_stopMarkerStatus;   // Флаг состояния маркера конца пакета
int sp_dataLength;     // Флаг состояния принимаемых данных
boolean sp_packetAvailable;  // Флаг завершения приема пакета
String TransmitData;                  //переменная для временного хранения передоваемых данных
String versionString;             // Строка, содержащая какую-нибудь бесполезную информацию :)
String Mask;                     //Маска для пирёма данных от ПК для турели
unsigned char string [10];       //Тоже самое что и sp_dataString просто в него не влазят все значения (только до 127 а здесть до 255)))), а работаю они паралельно.

int data_A2;                 //Переменные для приёма данных из строки протоколо обмена данными либо массива повторителя. 
int data_A3;

int pos_X_stab;               // Переменная для хранения углов от гироскопа для стабилизации
int pos_Y_stab;

bool stab_on = false;                // включает стабилизатор
bool zero;
bool stab_stop = false;

bool time_out = false;

unsigned long previousMillis;
unsigned long currentMillis;
long ttime_out = 200;


void sp_Reset()
{
  sp_startMarkerStatus = 0;  // Сброс флага маркера начала пакета
  sp_stopMarkerStatus = 0;  // Сброс флага маркера конца пакета
  sp_dataLength = 0;    // Сброс флага принимаемых данных
  sp_packetAvailable = false; // Сброс флага завершения приема пакета
}



void sp_ResetAll()
{
  for (int i ; i <= 10;)
{
  string[i] = 0;
  i++;
} 
  sp_dataString = "";    // Обнуляем буфер приема данных
  sp_Reset();     // Частичный сброс протокола
}


void sp_SetUp()
{
  sp_startMarker = "<@#>";   // Так будет выглядеть маркер начала пакета
  sp_stopMarker = "<&^>";  // Так будет выглядеть маркер конца пакета
  Mask = "SA000";            //Маска принемаемых с ПК данных. Так выглядит пакет в ктором передаются координаты сервы последнии 2а символа несут информацию координвтах курсора.
  sp_dataString.reserve(64);   // Резервируем место под прием строки данных
  sp_ResetAll();     // Полный сброс протокола
}
//команды для проверки протоколо обмена данными.
//<@#>ver?<&^>
//<@#>5Start<&^>
//<@#>EHex_input_value<&^> 
//<@#>4SA00<&^>     


void sp_Read()
{
  while(Serial.available() && !sp_packetAvailable)            // Пока в буфере есть что читать и пакет не является принятым
  {
    int bufferChar = Serial.read();                           // Читаем очередной байт из буфера
    
    if(sp_startMarkerStatus < sp_startMarker.length())        // Если стартовый маркер не сформирован (его длинна меньше той, которая должна быть) 
    {  
     if(sp_startMarker[sp_startMarkerStatus] == bufferChar)   // Если очередной байт из буфера совпадает с очередным байтом в маркере
     {
       sp_startMarkerStatus++;                                // Увеличиваем счетчик совпавших байт маркера
       
     }
     else
     {
       sp_ResetAll();                                         // Если байты не совпали, то это не маркер. Нас нае****, расходимся. 
     }
    }  
    else
    {
     // Стартовый маркер прочитан полностью
       if(sp_dataLength <= 0)                                 // Если длинна пакета на установлена
       {
       
         
       
       int hex = bufferChar;    // большой кастыль но работает)))
        int dec;
        
        //Serial.print (hex);
        //Serial.print ("\t");
        //Serial.print (dec);
            switch (hex)
            {
            case 48:
            dec = 0; 
            break;

            case 49:
            dec = 1; 
            break;

            case 50:
            dec = 2; 
            break;

            case 51:
            dec = 3; 
            break;

            case 52:
            dec = 4; 
            break;

            case 53:
            dec = 5; 
            break;

            case 54:
            dec = 6; 
            break;

            case 55:
            dec = 7; 
            break;

            case 56:
            dec = 8; 
            break;

            case 57:
            dec = 9; 
            break;

            default:
            sp_ResetAll();
            }


          //sp_dataLength = bufferChar;
         sp_dataLength = dec;                          // Значит этот байт содержит длину пакета данных
         dec = 0;
        
       }
      else                                                    // Если прочитанная из буфера длинна пакета больше нуля
      {
        if(sp_dataLength > sp_dataString.length())            // Если длинна пакета данных меньше той, которая должна быть
        {
          sp_dataString += (char)bufferChar;                  // прибавляем полученный байт к строке пакета

          int i = sp_dataString.length();
          
          string[i] = (unsigned char)bufferChar; 
           
          //Serial.print (sp_dataString.length());
           //Serial.print ("A");
          //Serial.print ("\t");
          //Serial.println (sp_dataLength  )
        }
        else                                                  // Если с длинной пакета данных все нормально
        {
          //Serial.print ("6");
          if(sp_stopMarkerStatus < sp_stopMarker.length())    // Если принятая длинна маркера конца пакета меньше фактической
          {
            if(sp_stopMarker[sp_stopMarkerStatus] == bufferChar)  // Если очередной байт из буфера совпадает с очередным байтом маркера
            {
              sp_stopMarkerStatus++;                              // Увеличиваем счетчик удачно найденных байт маркера
              if(sp_stopMarkerStatus == sp_stopMarker.length())
              {
                // Если после прочтения очередного байта маркера, длинна маркера совпала, то сбрасываем все флаги (готовимся к приему нового пакета)
                sp_Reset();    
                sp_packetAvailable = true;                        // и устанавливаем флаг готовности пакета
              }
            }
            else
            {
              sp_ResetAll();                                      // Иначе это не маркер, а х.з. что. Полный ресет.
            }
          }
          //
        }
      } 
    }    
  }
}

void sp_Send(String data)
{

  Serial.print(sp_startMarker);          // Отправляем маркер начала пакета
  Serial.print( data.length(), HEX );    // Отправляем длину передаваемых данных
  Serial.print(data);                    // Отправляем сами данные
  Serial.print(sp_stopMarker);           // Отправляем маркер конца пакета
}



void time_Out()
{   
     previousMillis = currentMillis;                         
}



void ParseCommand()
{
  if(sp_dataString == "ver?")   // Если была принята строка «ver?»
  {
   sp_Send(versionString);  // Отправляем на PC содержимое строки «versionString»
   
  }
  if(sp_dataString == "Start"){
    
  TransmitData = "ready";
  sp_Send(TransmitData);
    
  }

  if (sp_dataString.length() == Mask.length() && sp_dataString[0] == Mask[0] && sp_dataString[1] == Mask[1])

  {
     
    data_A2 = string [3];

//------------------------------------------// 
     int x, x_x;
      
      x = string [4];

      x_x = string [5];

      if ( x >= 255)
    {
    data_A3 = (( x + x_x ) - 180);
    }

    else
    {
      data_A3 = x;
    }
    stab_on = true;
    zero = true; 
    
  }
   if ( sp_dataString == "Stop" )
   {
    
    stab_on = false;
   }

   if ( sp_dataString =="stab_stop"  )
      {
        if (stab_stop == true)
        {
          stab_stop = false;
        }
        else 
        {
          stab_stop = true;
        }
      }
      


   


  
}
