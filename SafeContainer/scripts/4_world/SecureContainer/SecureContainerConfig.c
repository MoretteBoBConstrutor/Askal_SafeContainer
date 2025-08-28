class SecureContainerConfig
{
    // Configurações do sistema
    bool EnableSecureContainer = true;
    bool GiveToNewPlayers = true;
    bool RestoreOnDeath = true;
    
    // Restrições de itens (futuro)
    ref array<string> RestrictedItems;
    float MaxValueLimit = 0; // 0 = sem limite
    
    // Configurações do container
    string ContainerClassName = "Askal_SecureContainer";
    
    void SecureContainerConfig()
    {
        RestrictedItems = new array<string>();
        
        // ITENS PARA TESTE - Descomente para testar
        RestrictedItems.Insert("NVGoggles");
        RestrictedItems.Insert("Paper");
        RestrictedItems.Insert("M4A1"); // Para testar armas
        
        // TESTE: Bases de armas (irá bloquear todos os tipos)
        // RestrictedItems.Insert("RifleCore");
        // RestrictedItems.Insert("SniperRifle_Base");
        
        Print("[SecureContainer] Configuração inicializada - " + RestrictedItems.Count() + " restrições definidas");
    }
    
    // Salva configuração padrão
    void SaveDefault(string filePath)
    {
        JsonFileLoader<SecureContainerConfig>.JsonSaveFile(filePath, this);
    }
    
    // Carrega configuração
    void Load(string filePath)
    {
        if (FileExist(filePath))
        {
            JsonFileLoader<SecureContainerConfig>.JsonLoadFile(filePath, this);
        }
        else
        {
            SaveDefault(filePath);
        }
    }
    
    // Função para verificar se um item específico nao é permitido
    bool IsItemAllowed(string itemClassName)
    {
        // Verifica se está na lista de restritos
        for (int i = 0; i < RestrictedItems.Count(); i++)
        {
            string restrictedItem = RestrictedItems.Get(i);
            
            // Comparação exata ou por herança/contém
            if (itemClassName == restrictedItem || itemClassName.Contains(restrictedItem))
            {
                return false;
            }
        }
        
        return true; // Permitido
    }
    
    // Função para verificar limite de valor (implementação futura)
    bool IsWithinValueLimit(float itemValue)
    {
        if (MaxValueLimit <= 0)
            return true; // Sem limite
            
        return (itemValue <= MaxValueLimit);
    }
}
