//      HM-10 Name assignment (pulled from a HM10 name assigning program)
//      AT+NAME<yourName>enterKey
//      where <yourName> is the name you are assigning to
//      your HM10
//
//  Verify the new name of your HM10 by typing the following
//  command in the Send box of the monitor window:
//      AT+NAME?enterKey
//  The name now assigned to the HM10 will appear in the serial
//  monitor window.
//


/*                          How the ANCS variable system works 
 *             
 *     Useful guide: https://stackoverflow.com/questions/28488529/is-it-possible-to-use-hm-10-or-hm-11-ble-module-with-arduino-to-communicate-with
 *     (special thanks to Rafael Machado for providing such a well written answer!)
 *     
 *     Official guide (it's terrible  and didn't help me at all, but just in case you want it): 
 *     https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html
 * 
 * 
 * 
 * 
 *                             Understanding ANCS input
 *  Example input: AT+ANCS801012345                              
 *  It's split up as this: [AT+ANCS] + [8] + [0] + [1] + [01] + [2345]
 *  
 *  [AT+ANCS] is the way the module says that the data that's being received is from the ANCS process. Generally useless info.
 * 
 *   [8] is how long the data stream is (that is '01012345' here, right after the 8)
 * 
 *   [0] is the EVENT ID :
 *          0 -> EventIDNotificationAdded (a notification was added to the screen)
 *          
 *          1 -> EventIDNotificationModified  (a notification was modified)
 *          
 *          2 -> EventIDNotificationRemoved (a notfication was removed from the screen)
 *          
 *          3 ~ 255 -> Reserved EventID values (who tf knows, it's for special cases I think)
 *          
 *   
 *   [1] is the Category ID, tells us what type of notfication was sent.
 *          0 -> CategoryIDOther (used by most apps when they weren't assigned a category) Ex: rooted apps?
 *          
 *          1 -> CategoryIDIncomingCall (shows an incoming call)
 *          
 *          2 -> CategoryIDMissedCall  (shows a missed call) 
 *          
 *          3 -> CategoryIDVoicemail (shows a call was added to voicemail)
 *          
 *          4 -> CategoryIDSocial (shows a social media app made a notification) Ex: Discord
 *          
 *          5 -> CategoryIDSchedule (shows a schedules event notificaiton was on screen)
 *          
 *          6 -> CategoryIDEmail (shows an email notification was added to the screen)
 *          
 *          7 -> CategoryIDNews (shows a news notification was added to the screen)
 *          
 *          8 -> CategoryIDHealthAndFitness (shows a fitnesss app notification was added to the screen)
 *          
 *          9 -> CategoryIDBusinessAndFinance (shows a buissness and finance not was added to screen)
 *          
 *          10 -> CategoryIDLocation (shows a location app notification?)
 *          
 *          11 -> CategoryIDEntertainment (shows entertainment notifications) Ex: youtube?
 *          
 *          12~255 -> Reserved CategoryID values (I have no idea again. Could be anything.)
 *          
 *  [01] is the cataegory count
 *          Basically is a counter for how many notifications are onscreen.
 *          
 *  [2345] is what's called the NotificationUID       
 *          It's basically an address for the notification, which can be called on with a command to get important info about the notification
 *          such as the app name, the message itself, the time sent, the message size, etc.
 *          
 *          
 *                           Asking the module for notification attributes:
 *       Format: AT+ANCS2345100                 
 *       Split up: [AT+ANCS] + [2345] + [1] + [00]
 *       
 *  [AT+ANCS] - this tells the module that you want to access the ANCS system.      
 *  
 *  [2345] - this is the NotificationUID from the ANCS input that was received.
 *  
 *  [1] - this number controlls what information you want from the notification 
 *        0 -> NotificationAttributeIDAppIdentifier (this sends back the name of the app) Example: For a discord ping it gives 'discord'
 *        
 *        1 -> NotificationAttributeIDTitle  (this sends back the 'title' of the notification, AKA the wording in bold) Example: for discord it's the name of who or what pinged you
 *        
 *        2 -> NotificationAttributeIDSubtitle (this sends back the 'subtitle' of the notification, AKA the wording not in font) Example: for discord it's the message itself I think
 *        
 *        3 -> NotificationAttributeIDMessage (this sends back the 'message' of the notification
 *        4 ->
 *        5 ->
 *        6 ->
 *       
 */

#include <SoftwareSerial.h>
#include <avr/wdt.h>
const byte rxPin = 10;
const byte txPin = 9;

//this var will change depending on what type of information the code is asking the Bluetooth module 
//(so it can change the char size and get the correct info)

  //0 -> Normal mode - awaiting a notification
  //1 -> Name aquireing mode (gets the app name)
  //2 -> Title mode  (gets the contents of the app 'title')
  //3 -> Content mode  (gets contents of message)

SoftwareSerial BLEserial (rxPin, txPin);  // Declare BLE serial pins

//reserve of char values used for gathering data from modules



void setup()
{
  
  MCUSR = 0;  // clear out any flags of prior resets.
  //basic information setup, including file name and date started.
  Serial.begin(9600);
  BLEserial.begin(9600);
  //Serial.print("Sketch:   ");   Serial.println(__FILE__);
  //Serial.print("Uploaded: ");   Serial.println(__DATE__);
  //Serial.println(" ");
  Serial.print("\n\n ******IOS ANCS v4.4 Online!******"); 
}


  //gets the UID of the notification
  String Notification_UID;
  //stores response into a string for ease of access
  String full_answer = "";
  int Resetting_Counter=0; //for some reason every third time the system fucking dies, so this counters this.


  
//--------------------------------------------------------Main Loop (handles everything)----------------------------------------------------------------------------
void loop()
{
  String APP_NAME="";
  String APP_CONTENTS ="";
  String APP_TITLE = "";
  String APP_SUBTITLE ="";
  
  int Response = 0;
  char c = ' ';
  char e = ' ';
  char f = ' ';
  char g = ' ';  
  char h = ' '; 
  
  //set up 16 bit char (NOTE, 16 bits is ideal for gathering the notification info ONLY)
  char ANCS_8Buffer[16];
  
  
  char Notification_Type; //this will hold the var which says what type of notification was received. (see line 14)
  char Notification_Status; //this will hold if the notification is new, edited, or deleted.
          
  // Reading from Bluetooth module and sending to the Arduino Serial Monitor
  if (BLEserial.available()) //check if serial is connected.
  {
    Resetting_Counter++;
    if ((Response == 0) || (Response == 4))  //0 is DEFAULT CONDITION , 4 is user override
    {
      c = BLEserial.readBytes(ANCS_8Buffer, 16); //save input to 16 bit char 
      //Serial.print("Buffer contents: " + String(ANCS_8Buffer));//print resulting 16 bit char
      //Serial.print("\n -----Notification Buffer Complete.----- \n");

      Serial.println("####  New Notification Detected  ####"); //this print indicates a new notification is being processed
        
      //sometimes data gets repeated accidentally, but uses ON instead of AT. This detects those instances and ignores them.
      if (String(ANCS_8Buffer).indexOf("ON") > 0)
      {
        Serial.println("Invalid data. Ignoring...");
        Response = 0;
      }      
      
      //checking if it was user override
      if (Response !=4)
      {
        Response = 1; //moves to the next stage, getting notification attributes.
      }
      else
      {
        Serial.println("$$$$$$$$$$$$    USER OVERRIDE DETECTED.   $$$$$$$$$$$$");
        Response =3;
      }

          //storing the Notification_UID into a useable string (needs the last 4 numbers)
      Notification_UID = (String(ANCS_8Buffer[12])+ String(ANCS_8Buffer[13])+String(ANCS_8Buffer[14])+String(ANCS_8Buffer[15]));
      Serial.print("Gathered UID (must be 4 bit number): " + String(Notification_UID) + "\n" + "\n");
    }

//-----------------------------Response mode 1 - awaiting the response notification to get the name of the app responsible.
    if (Response == 1) //NAME GET CONDITION 
    {     
      //getting the type of notification:

        //grabs the notification type from the buffer (it's the 8th char)
        Notification_Type= ANCS_8Buffer[9];
        Notification_Status= ANCS_8Buffer[8];
        //go to a method which checks and prints the type of notification it is
        ID_Type(Notification_Type);
        
        //Serial.println("Notification Status (0 is new, 1 is edited, 2 is removed): "  + String(Notification_Status));
       
      //getting the name of the application:
      if (Notification_Status == '0')
      {
        Send_Request(0,"00"); //Send a request for information
        Response_String_Condenser(); //condense the input into one global string called 'full_response'
        
        //Detect the name of the app from the given 'full_response'
        String App_Detectable_database[] //this holds the words that consistently show up in a full answer to indicate an app
        {
          "iscord",     //Discord
          "bilephone",  //phone
          "WhatsApp",   //Whatsapp
          "bileSMS",    //SMS
          "mail",       //Gmail
          "eddit",      //Reddit
          "Tweet",      //Twitter      
          "youtub",     //Youtube
          "picab",       //Snapchat
          //this stores the actual name of the app
          "Discord",        //Discord
          "Phone Call",     //phone
          "WhatsApp",       //Whatsapp
          "Text Message",    //SMS
          "Gmail",          //Gmail
          "Reddit",         //Reddit
          "Twitter",        //Twitter     
          "Youtube",        //Youtube
          "Snapchat"        //Snapchat          
        };     
      
        int Amount_of_apps = 9; //this is how many apps the system knows exist.

        //this system cylces through the dataset above to find the keywords which indicate which app name hidden in the full_response string
        for (int i=0;i<Amount_of_apps;i++)
        {
          //Serial.println("Checking for precense of word: " + App_Detectable_database[i]); 
          if(full_answer.indexOf(App_Detectable_database[i]) > 0)
          {
            APP_NAME = App_Detectable_database[i+Amount_of_apps];
            Serial.println("\n" + App_Detectable_database[i+Amount_of_apps]+ "\n");
            break;
          }
        } 
        ///clear full answer:
        full_answer = "";
        
      }
      else
      {
          //check if the notification was deleted.
          if(Notification_Status == '2')
          {
            wdt_enable(WDTO_15MS); // turn on the WatchDog and don't stroke it.
            for(;;) { 
              // do nothing and wait for the eventual...
            } 
          }
      }
      
      Response=2;      
    }
//-----------------------------Response mode 2 - awaiting the response notification to get the title of the notification.    
    if (Response == 2) //TITLE GET CONDITION 
    {
      //getting the title of the application:
      if (Notification_Status == '0')
      {
        Send_Request(1,"99"); //Send a request for information
        int titledata_counter=0;
        String Title_info= "";
        
        while(titledata_counter <= 75)
        {
          if (BLEserial.available())
          {
              f = BLEserial.read();
              if(titledata_counter > 33)
              {
                Title_info += String(f);
              }
              
          }
          delay(10);
          titledata_counter++;         
        }
        
        APP_TITLE = String(Title_info);
        Serial.println("\n" + String(Title_info) + "\n"); 
        Title_info = "";      
        ///clear full answer:
        full_answer = "";  
      }      
      Response=3;
    }    
//-----------------------------Response mode 3 - awaiting the response notification to get the contents of the notification.
    if (Response == 3) //CONTENT GET CONDITION
    {
     
      //getting the message of the application:
      if (Notification_Status == '0')
      {
        
        Send_Request(2,"99"); //Send a request for information
        int messagedata_counter_sub=0;
        String message_info_sub= "";
        
        while(messagedata_counter_sub <= 75) //75
        {
          
          if (BLEserial.available())
          {
              g = BLEserial.read();
              if(messagedata_counter_sub > 33) //33
              {
                message_info_sub += String(g); 
              }
              
          }
            delay(10);
            messagedata_counter_sub++;   
        }
        messagedata_counter_sub=0;
        
        //Serial.println("\n" + String(message_info) + "\n");       
        String Subtitle_Message = String(message_info_sub);
        
        Subtitle_Message.replace("OK+ANCS:14","");
        Subtitle_Message.replace(" ","");

        //special cases (keep to a bare minimum for small details)
        Subtitle_Message.replace("Incoming CalOK+ANCS:01l","Incoming Call");
        
        APP_SUBTITLE = String(Subtitle_Message);
        Serial.println("\n" + Subtitle_Message + "\n"); 
      }      
    } 
    if(Response = 4)
    {
       if (Notification_Status == '0')
      {
        Send_Request(3,"99"); //Send a request for information
        int messagedata_counter=0;
        String message_info= "";
        
        while(messagedata_counter <= 75) //75
        {
          
          if (BLEserial.available())
          {
              h = BLEserial.read();
              if(messagedata_counter > 33) //33
              {
                message_info += String(h); 
              }
              
          }
            delay(10);
            messagedata_counter++;   
        }
        messagedata_counter=0;
        
        //Serial.println("\n" + String(message_info) + "\n");       
        String Final_Message = String(message_info);
        
        Final_Message.replace("OK+ANCS:14","");
        Final_Message.replace(" ","");
  
        //special cases (keep to a bare minimum for small details)
        Final_Message.replace("Incoming CalOK+ANCS:01l","Incoming Call");
        
        APP_CONTENTS = String(Final_Message);
        Serial.println("\n" + Final_Message + "\n");        
  
         //whenever there's no data or a deleted notification, the sys jumps here and resets.
        //Clearing vars for next notification
        message_info = "";
        ///clear full answer:
        full_answer = "";
        //clear input buffer
        ANCS_8Buffer[0] = 0;
        //once all the info is gathered, it clears both char arrays so the next notification has a clean slate.
        Response=0;
        Serial.println("--Notification Done--");    
      }
    }
  }//end of if BLE.avalible()


//----killing and resetting the system-----

  //if (Resetting_Counter >1)
  //{
  //  wdt_enable(WDTO_15MS); // turn on the WatchDog and don't stroke it.
  //  for(;;) { 
  //    // do nothing and wait for the eventual...
  //  } 
  //}




//--------------Read from the Serial Monitor and send to the Bluetooth module----------
  if (Serial.available())
  {
    e = Serial.read();
    Serial.write(e);
    

    // do not send line end characters to the HM-10
    if (e != 10 && e != 13 )
    {
      BLEserial.write(e);
      Response=4;  //enable user override
    }
    
  }

}//end void loop

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------gets the notification type.------------------------------------------------------------------
void ID_Type(char Type_ID)
{
  Serial.print("Notification Type: ");
  switch (Type_ID)
  {
    case '0':
      Serial.print("Other\n");
      break;
    case '1':
      Serial.print("Incoming Call\n");
      break;
    case '2':
      Serial.print("Missed Call\n");
      break;
    case '3':
      Serial.print("Voicemail\n");
      break;
    case '4':
      Serial.print("Social\n");
      break;
    case '5':
      Serial.print("Schedule\n");
      break;
    case '6':
      Serial.print("Email\n");
      break;
    case '7':
      Serial.print("News\n");
      break;
    case '8':
      Serial.print("Health\n");
      break;
    case '9':
      Serial.print("Buss\n");
      break;
    case 'A':
      Serial.print("Location\n");
      break;
    case 'B':
      Serial.print("Entertainment\n");
      break;
  }
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------general purpose system meant to SEND data to module.------------------------------------------------------
void Send_Request(int Type,String Amount)
{
  String sendS = "";   
  //Serial.println("Received ID: " + String(Notification_UID));
  sendS = ("AT+ANCS" + Notification_UID + String(Type) + String(Amount));
  
  //Serial.println("Initial String Request: "+sendS + "\n");  
  char sendChar[sendS.length()+1];
  
  sendS.toCharArray(sendChar,sendS.length()+1);

  //Serial.println("Char Array: " + String(sendS.length()));
  //Serial.println("BLE Command: "+String(sendChar)+ "\n");
            
  BLEserial.write(sendChar); //send to BLE
  sendChar[0] = 0; //clear sendChar
  sendS = ""; //clear sendS
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------Gathers the entire response from the module, then condenses it into a long string for ease---------------------------------------
void Response_String_Condenser()
{
  int no_connection=0;
  int looping=0;
    //set up char to get name from response buffer
  char ANCS_ResponseBuffer[16];
  char d = ' ';
  
  while(looping <= 100)
  {
    if (BLEserial.available())
    {
      //Serial.println("Counter: " + String(looping)+"\n");  
          //get data from module
      delay(2);
      d = BLEserial.readBytes(ANCS_ResponseBuffer, 16); //save input to 16 bit char 
      

      //Serial.println("ANCS_ResponseBuffer: " + String(ANCS_ResponseBuffer));
      
          //append data to a string
      full_answer += ANCS_ResponseBuffer;            
          //increment loop counter
      looping+=1;
          //clear the namebuffer so the next data can be gathered.
      ANCS_ResponseBuffer[0] = 0;
    }
    else
    {
          //if the system is spamming no connection, then the data was lost forevor (or there is no more data to gather)
      if (no_connection < 30)
      {  
        //Serial.println(" \n ERR: BLE not availible ("+String(no_connection)+"/30) attempts.");
        no_connection++;
        delay(2);
      }
      else
      {
        //Serial.println(" \n ERR: CONNECTION FAILED/COMPLETE (no way to tell really, but the data stream is gone) \n");  
        break; //kill the loop
      }
      
    } 
  }
  //Serial.print("Full extracted answer: " + full_answer  + "\n");  //this is the full extraced data from the BLE module which is filtered through to get attribute information (name of the app, message, etc)
      //reset vars
  no_connection =0;
  looping=0;
  ANCS_ResponseBuffer[0] = 0; //clear ANCS_ResponseBuffer
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
