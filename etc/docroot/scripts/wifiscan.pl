#!/usr/bin/perl
#############################################################################
# Author: Timothy Mills
# Date  : 8 - 4 - 08
# File  : wiscan.pl
#
# wiscan.pl takes the output of 'iwlist scan' and makes it in an easier
# to read format for those of us who still deal with the terminal :)
#############################################################################

#Run the command and suppress error output
$iwlistOut = `iwlist scan 2> /dev/null`; 

@myLines = split("\n", $iwlistOut);
@cellArr = [];

$currNum = 0;
varInit();

foreach (@myLines)
{
    if ($_ =~ "Cell [0-9][0-9]")
    {
		if ($currNum > 0)
		{
			cleanUpCurrentWifi();
			printOneLine($essid, $quality, $sigLvl, $channel, $mode, $encryption, $auth);
			varInit();
		}
		$currNum++;
    }

    $cellArr[$currNum] = $cellArr[$currNum]."$_"."\n";

	if($_ =~ "Channel:")
    {
		handleChannel($_);
    }
	elsif($_ =~ "Quality=[0-9]+/[0-9]+")
    {
		handleQuality($_);
    }
	elsif($_ =~ "Encryption key")
    {
		handleKey($_);
    }
    elsif ($_ =~ "ESSID")
    {
		handleESSID($_);
    }
	elsif($_ =~ "Mode")
    {
		handleMode($_);
    }
    elsif($_ =~ "IE: WPA .+ [0-9]")
    {
		handleAuth($_);
    }
	elsif($_ =~ "IE: IEEE 802.11i/WPA2")
    {
		handleAuth($_);
    }
	elsif($_ =~ "Pairwise Ciphers")
    {
		handleCipher($_);
    }   
    elsif($_ =~ "Authentication Suites")
    {
		handleAuthSuite($_);
    }
}


if ($currNum > 0)
{
	#last one
	cleanUpCurrentWifi();
    printOneLine($essid, $quality, $sigLvl, $channel, $mode, $encryption, $auth);
}
else
{
    print("No scan results\n");
}

###########################################################################
# Sub functions
###########################################################################

sub printOneLine()
{
	$_[0] =~ s/&/&amp;/g;	$_[0] =~ s/\</&lt;/g;	$_[0] =~ s/\>/&gt;/g;	$_[0] =~ s/\"/&quot;/g;
	#$_[0] =~ s/\'/&apos;/g;
    printf("<wifi><ssid>%s</ssid><qty>%s</qty><lvl>%s</lvl><ch>%s</ch><mode>%s</mode><encryption>%s</encryption><auth>%s</auth></wifi>\n",$_[0], $_[1], $_[2], $_[3], $_[4], $_[5], $_[6]);
}

sub varInit()
{
    $essid = "";
    $quality = "";
    $sigLvl = "";
    $keyonoff = "";
    $auth = "";
	$authSuite = "";
    $encryption = "";
	$cipher = "";
	$mode = "";
	$channel = "";
}

sub cleanUpCurrentWifi ()
{
    if ($keyonoff eq "on")
	{
		if ($auth eq "")
		{
			$encryption = "WEP";
			$auth = "WEPAUTO";
		}
		else
		{
			$encryption = $auth;			#WPA, WPA2
			if($cipher =~ /CCMP/)
			{
				$encryption = "AES";		#Change encryprtion to AES
			}
			$auth = $auth . $authSuite;		#WPAPSK, WPAEAP, WPA2PSK, WPA2EAP
		}
	}
	else
	{
		$encryption = "NONE";
		$auth = "OPEN";
	}
}

###########################################################################

sub handleCipher ()
{
    $cipher = (split(":", $_[0]))[1];
}

sub handleQuality ()
{
    $qual = $_[0];

    @qualArr = split(" ", $qual);

    $quality = (split("=", $qualArr[0]))[1];
    $sigLvl = (split("=", $qualArr[2]))[1];
}

sub handleKey ()
{
	#Eg.: Encryption key:on
    $keyonoff = (split(":", $_[0]))[1];
}

sub handleESSID ()
{
	#Eg.: ESSID:"ChumbyWPA"
    $essid = (split(":", $_[0]))[1];
	$essid =  substr $essid, 1, -1;
}

sub handleAuth ()
{
	if($_ =~ "IE: WPA")
    {
		$auth = "WPA";
    }
	elsif($_ =~ "IE: IEEE 802.11i/WPA2")
    {
		$auth = "WPA2";
    }
}

sub handleAuthSuite ()
{
	#Eg.: Authentication Suites (1) : PSK
    $authSuite = (split(" ", $_[0]))[4];
}

sub handleMode ()
{
    $mode = (split(":", $_[0]))[1];
}

sub handleChannel ()
{
    $channel = (split(":", $_[0]))[1];
}