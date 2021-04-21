A plant soil moisture management application using Arduino MKR 1000, moisture sensor and 5V pump.

This project contains the Arduino sketch in the [arduino folder](https://github.com/kleag/moisture/tree/main/arduino/moisture_lcd) and the [Django REST framework-based API](https://github.com/kleag/moisture/tree/main/). There is also example configuration files to deplay the application using nginx and gunicorn in [etc](https://github.com/kleag/moisture/tree/main/etc). It is up to you to learn how to deploy a Web service using Django, nginx and gunicorn but feel free to open issues if you have questions, suggestions or (constructive critics) on this project itself.


![Fritzing breadboard schema](https://raw.githubusercontent.com/kleag/moisture/main/Fritzing/Moisture_bb.png)


This project was realized using [this kit available from Amazon](https://www.amazon.fr/gp/product/B0814HXWVV/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1). I had two problems using it. Firstly, the relay was open when HIGH on the Arduino pin and closed when LOW. Secondly, the pump was creating important electric perturbations, causing strange characters to appear on the screen and changes on the analog moisture measures.

Thanks to [this page](https://arduinodiy.wordpress.com/2018/08/07/re-inverting-an-inverting-relay/) and the [Electronique/Dépannage/Mécanique/Physique/Chimie/Informatique/Modélisme](https://www.facebook.com/groups/168734660482761/) Facebook group (thanks Olivia!), I was able to make it work as I wanted. You can see on the upper right the filtering part and on the lower right the transistor-based inverser.

After testing on the above breadboard, I made this perfboard schema:

![Fritzing perfboard schema](https://raw.githubusercontent.com/kleag/moisture/main/Fritzing/Moisture-perfboard_bb.png)

I made some little changes during the soldering itself and got this result:

  - ![Up of the soldered project](https://raw.githubusercontent.com/kleag/moisture/main/up.jpg)
  - ![Down of the soldered project](https://raw.githubusercontent.com/kleag/moisture/main/down.jpg)

