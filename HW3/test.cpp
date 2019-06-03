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



Process Cashier(){
    while(true){
        BARTER.nextCustomer()
        BARTER.process()
        BARTER.endTransaction()
    }
}


Monitor BARTER{
    DEALER{
        TICKET ticket
        STUFF product
        DOLLAR amount
        DEALER DEALER(ticket, product, price){
            DEALER newDealer
            newDealer.ticket = ticket
            newDealer.product = product
            newDealer.amount = price
            return newDealer
        }
    }
    BUYER{
        TICKET ticket
        DOLLAR amount
        BUYER BUYER(ticket,amount){
            BUYER newBuyer
            newBuyer.ticket = ticket
            newBuyer.ammount = amount
            return newBuyer
        }
    }
    TRANSACTION{
        TICKET ticket
        DOLLAR amount_seller
        DOLLAR amount_buyer
        STUFF product
        boolean seller_in = False
        boolean buyer_in = False
        matchedTicket = False

        TRANSACTION TRANSACTION(ticket,stuff,amount){
            TRANSACTION newTransaction
            newTransaction.ticket = ticket
            newTransaction.stuff = stuff
            newTransaction.amount_seller = amount
            boolean seller_in = True
        }
        TRANSACTION TRANSACTION(ticket,amount){
            newTransaction.ticket = ticket
            newTransaction.amount_buyer = amount
            boolean buyer_in = True
        }
    }
    TRANS_MAP{
        map<TRANSACTION> transactions[large_prime]
        void insertSeller(ticket,product,amount){
            int i = getHash(ticket)
            transactions[i] = TRANSACTION(ticket,product,amount)
        }
        void insertBuyer(ticket,amount){
            int i = getHash(ticket)
            transactions[i] = TRANSACTION(ticket.amount)
        }
        boolean getIndex(ticket){
            int i = getHash(ticket)
            if(transactions[i]==NULL)
                return -1
            else
                return i
        }
        boolean both_in(ticket){
            int i = getHash(ticket)
            return (transactions[i].buyer_in && transactions[i].seller_in)
        }
        DOLLAR getMoney(ticket){
            i = getIndex(ticket)
            DOLLAR money = transactions[i].amount
            transactions[i].amount = $ZERO
            if(transactions[i].product==NULL){
                transactions[i] = NULL
            }
            transactions[i].buyer_in = False
            return money
        }
        STUFF getProduct(ticket){
            i = getIndex(ticket)
            STUFF product = transactions[i].stuff
            transactions[i].stuff = NULL
            if(transactions[i].amount==NULL){
                transactions[i] = NULL
            }
            transactions[i].seller_in = False
            return product
        }
    }
    queue<ticket> tickets
    queue<int> tickets_index
    int cashiers
    TRANS_MAP transactions
    DOLLAR POT
    ticket newBuyer(ticket,amount){
        transactions.insertBuyer(ticket,amount)
        return transactions.getIndex(ticket)
    }
    ticket newSeller(ticket,amount){
        transactions.insertSeller(ticket,product,price)
        return transactions.getIndex(ticket)
    }
    boolean checkTicket(ticket){
        return transactions.checkTicket(ticket)
    }
    STUFF getProduct(ticket){
        return transactions.getProduct(ticket)
    }
    DOLLAR getMoney(ticket){
        return transactions.getMoney(ticket)
    }


    STUFF BUY(ticket,amount){
        while(cashiers==0) wait(cashier_available);
        tickets.push_front(newBuyer(ticket,amount))
        cashiers = cashiers - 1
        notify(newCustomer)
        while(checkTicket(ticket)){
            wait(matchedTicket)
        }
        STUFF product = getProduct(ticket)
        notify(completeTransaction)
        return product
    }
    DOLLAR SELL(ticket,product,price){
        while(cashiers==0) wait(cashier_available);
        tickets.push_front(newSeller(ticket,product,price))
        cashiers = cashiers - 1
        notify(newCustomer)
        while(!checkTicket(ticket)){
            wait(matchedTicket)
        }
        DOLLAR money = getMoney(ticket)
        notify(completeTransaction)
        return money
    }
    nextCustomer(){
        cashier = cashier + 1
        notify(casher_available)
        wait(newCustomer)
    }
    process(){
        for(int i=0,i<len(tickets),i++)){
            ticket = tickets[i]
            if(transactions.both_in(ticket)){
                int i = transactions.getIndex(ticket)
                if(transactions[i].amount_buyer==transactions[i].amount_seller){
                    POT = POT + .1 * transactions[i].amount_seller
                    transactions[i].amount_seller = .9 * transactions[i].amount_seller
                    transactions[i].matchedTicket = True
                    tickets_index.push_front(i)
                    continue
                }
                else{
                    POT = POT +transactions[i].amount_seller
                    transactions[i].amount_seller = $ZERO
                    transactions[i].product = NOTHING
                    transactions[i].matchedTicket = True
                    tickets_index.push_front(i)
                    continue
                }
            }
        }
    }
    endTransaction(){
        notify(matchedTicket)
        wait(completeTransaction)
        int i = tickets_index.front()
        tickets_index.pop()
        tickets[i].remove()
    }
}