function register()
{
    var in_name = window.prompt("Enter your name: ");
    alert("Your name is "+name+"Can't be changed.");
    var Booking={
        id:1000,
        name:in_name,
    }
    Booking['id']=Booking['id']+1;
    document.getElementById("register").innerHTML = "Registration id: "id+"Name: "+name;
              	}