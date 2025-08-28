class SecureContainerData
{
    string OwnerUID;
    ref array<ref SecureItemData> StoredItems;
    
    void SecureContainerData(string uid)
    {
        OwnerUID = uid;
        StoredItems = new array<ref SecureItemData>();
    }
}

class SecureItemData
{
    string ClassName;
    float Health;
    float Quantity;
    int Row;
    int Col;
    bool IsFlipped;
    
    // Para itens com munição
    bool HasAmmo;
    int AmmoCount;
    
    // Para itens comestíveis
    bool IsFood;
    int FoodStageValue;
    
    // Para attachments (futuro)
    ref array<ref SecureItemData> Attachments;

    //✅ NOVA FUNÇÃO: para Escrever Itens dentro de Containers
    bool HasInventory;                          // Para detectar se item tem inventário
    ref array<ref SecureItemData> NestedItems;  // Para itens dentro de outros itens
    
    bool IsContainerAttachment;                 // Flag para identificar attachments do container
   

    void SecureItemData()
    {
        Attachments = new array<ref SecureItemData>();
        HasAmmo = false;
        IsFood = false;
        Health = 1.0;
        Quantity = 0;
        Row = 0;
        Col = 0;
        IsFlipped = false;
        //✅ NOVA FUNÇÃO: para Escrever Itens dentro de Containers
        HasInventory = false;
        NestedItems = new array<ref SecureItemData>();
        IsContainerAttachment = false;
    }
}