s.value = 1;
P(s){
    if(s.value==1){
        s.value = 0;
    }
    else{
        s.queue.push_back(process)
        process.block()
    }
};
Update(FILE);
V(s){
    if(s.queue.empty()){
        s.value = 1;
    }
    else{
        process = s.queue.front();
        process.wakeup();
    }
};


process CUSTOMER[1 to N]
{
   while (true) {
     let_your_hair_grow();
     call Barber_Shop.get_haircut();
   }
}

process BARBER
{
   while (true) {
     call Barber_Shop.get_next_customer();
     haircut();
     call Barber_Shop.finished_cut();
   }
}

The monitor from fig 5.10 ripped off to bare necessities:
=======================================================

monitor Barber_Shop
{
  int barber = 0; # chair and  open variables are not needed
  cond barber_available;
  cond chair_occupied;
  cond door_open;
  cond customer_left;

  procedure get_haircut()  
  {
     while (barber==0) wait(barber_available);
     barber = barber – 1;  
     signal(chair_occupied);
     wait(door_open);
     signal(customer_left);
  }
    
  procedure get_next_customer()  
  {
     barber = barber + 1; signal(barber_available);
     wait(chair_occupied);
  }
    
  procedure finished_cut()  
  {
     signal(door_open);
     wait(customer_left);
  }
} # monitor Barber_Shop


Monitor OIL{
    Total = 0
    Full
    Empty = True
    TruckCap
    DepotCap

    Deposit(contents){
        if (Total == DepotCap){
            wait(Full)
        }
        tank.push(contents)
        Total+=contents
        if Total >= TruckCap{
            notify(Empty)
        } 
    } 

    Retrieve(){
        if tank <= TruckCap{
            Wait(Empty)
        }
        Total-=TruckCap
        Amount = TruckCap
        if Total == DepotCap
            notify(Full)
        }
        Return Amount
}
