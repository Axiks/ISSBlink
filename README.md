<p align="center">
    <!-- <img width="200" src="https://openclipart.org/download/196091/satellite-icon.svg" alt="Logo"> -->
    <img width="256" src="https://user-images.githubusercontent.com/36519646/231486464-26f28978-ee64-42d2-becd-58396012fc1d.png" alt="logo">
</p>
<h1 align="center">ISS Blink </h1> 

![Arduino](https://img.shields.io/badge/-Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)

Notifies about the arrival of the ISS over you

## Who is this project for?
For those who would like to watch the ISS from Earth :earth_africa:

## How can you see the ISS?
From Earth, it is visible as a point that moves quickly across the sky and gradually changes its brightness

<img src="https://user-images.githubusercontent.com/36519646/231487707-618c90d5-8c8a-4d43-a576-20ed0485cd26.jpg" alt="Iss at night sky">

> The ISS can be seen as a bright moving point

[![ISS](https://img.youtube.com/vi/yGwTqG03pHM/0.jpg)](https://www.youtube.com/watch?v=yGwTqG03pHM)


> This video shows how the ISS looks like from the surface of the earth

## When can I see the ISS?
For this, there are special programs and services that help in this. But all of them require direct interaction with them.
To solve this problem, a circuit based on the ESP 32 microcontroller was developed, and software was written for it.
This device can simply be placed on the table and it will work 24/7 :clock830:

## How the ISS tracker works?

This device, using a LED or OLED screen, informs about the time remaining until the International Space Station becomes visible in the sky.

<img width="512" src="https://user-images.githubusercontent.com/36519646/231486888-8345531e-cd9c-478c-945e-39070e12b0db.JPG" alt="Material Bread logo">

*LED indicator*

<img width="512" src="https://user-images.githubusercontent.com/36519646/231488231-ac6158ed-3e43-4d73-ae90-aaaba36d8406.jpg" alt="LED indicator">

> 30 minutes before the start of ISS visibility, the indicator starts flashing slowly and remains continuously lit during the visibility period.

*OLED display*

<img width="512" src="https://user-images.githubusercontent.com/36519646/231488488-878eaa56-60a1-4bc4-af62-6c11c19f8287.jpg" alt="OLED display">

> Displays the time remaining until the appearance of the ISS in the field of view.

<img height="512" src="https://user-images.githubusercontent.com/36519646/231488823-732788b0-b3ad-4dd5-99f6-f91c9de59667.jpg" alt="Material Bread logo">

> We also display the local time and the number of upcoming ISS appearances within a few days (adjustable).
> **1** - local time
> **2** - the number of upcoming ISS appearances

<img height="512" src="https://user-images.githubusercontent.com/36519646/231488995-93b82a33-103e-414b-abf7-6570ed4ee79f.jpg" alt="Material Bread logo">

> If the ISS will not fly by within a few days, we show a corresponding message

## First launch
Step 1. During the first launch, we need to connect the board to the Internet using WiFi
1. We supply power to the board

<img width="512" src="https://user-images.githubusercontent.com/36519646/231489151-e93a504f-31f2-498d-8c96-94978336ed12.jpg" alt="Material Bread logo">

2. We are looking for a Wi-Fi network called ISS

<img height="64" src="https://user-images.githubusercontent.com/36519646/231489326-4e87fb1f-2615-4aa1-aace-cdb8abc4d129.jpg" alt="Material Bread logo">

3. We join her. The tracker automatically redirects to the settings page

<img width="512" src="https://user-images.githubusercontent.com/36519646/231758842-024043c0-3622-45d5-9347-ce5beb638ffc.png" alt="Material Bread logo">

4. To continue the configuration, you need to click on the link "*configure page*".

<img height="64" src="https://user-images.githubusercontent.com/36519646/231758903-9d80ed92-e30c-428d-b4fe-1a19661da764.png" alt="Material Bread logo">

5. Here we enter the name(1) and password(2) for our home Wifi network. You also need to come up with a password for future access to the tracker

<img width="512" src="https://user-images.githubusercontent.com/36519646/231758973-c96902c5-dec9-4fb7-b150-48c3199bb3e8.jpg" alt="Material Bread logo">

Step 2. In the next step, you need to submit your location
1. Enter the coordinates of your location

<img width="512" src="https://user-images.githubusercontent.com/36519646/231759363-671017a1-5b0a-45c6-bcf2-ddb9742e6d38.jpg" alt="Material Bread logo">

> They can be determined, for example, using the service https://earthexplorer.usgs.gov/

Step 3. Save the settings

<img width="512" src="https://user-images.githubusercontent.com/36519646/231759424-4060927a-1360-4136-8b09-fdfcae759ed2.png" alt="Material Bread logo">

**The device will reboot and start working!** :satellite:

### Secondary functions
#### Change settings
You can always change the settings of the tracker after its initial setup. To do this, you need to access the device's IP address.
<img width="512" src="https://user-images.githubusercontent.com/36519646/231759478-11689a78-98ec-4b73-a0b3-b564758495c1.jpg" alt="Material Bread logo">

> To access the settings, use the username "admin" and the password you created in the previous step (step 1.5).

## The initial prototype of the device design

*In development*

<img width="512" src="https://user-images.githubusercontent.com/36519646/231759523-60e42196-cbb8-41d9-9508-d572e43e6317.png" alt="Material Bread logo">
<img width="512" src="https://user-images.githubusercontent.com/36519646/231759593-9bacddd7-9c4d-42d0-a3dc-cfd9ae31b370.png" alt="Material Bread logo">
