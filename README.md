# rozhlas
Riadenie zosilňovačov cez RS485 modem

použitie:

rozhlas SERIOVY_PORT ON|OFF

napr.

rozhlas COM3 on  
rozhlas COM3 off  

komunikuje cez rozhranie RS485 v móde MODBUS s doskou so 4ks riadených relé:  
 - eletech CE040 4 channel rs485 relay board  
 https://www.ebay.com/itm/272104454591  
 https://www.ebay.com/itm/186109384365  
 https://autokomplexbardejov.sk/value_410956-Store  
 https://www.ebay.com/itm/375733618228  
 
dokáže buď zapnúť alebo vypnúť všetky 4 kanály  
  
Z PC je RS485 linka riadená cez USB modem:  
  https://techfun.sk/produkt/prevodik-usb-na-rs485/  
  
Protokol:  
  
https://www.laskakit.cz/user/related_files/8ch_rele_rs485_protocols.pdf  
  
  funkcia 6 prepína: 1 - zapnúť, 2 - vypnúť  

Program beží na OS Windows, ale po výmene modulu *serial* by mal fungovať aj pod inými OS.  
Cenada 2025, pavel.petrovic@gmail.com
