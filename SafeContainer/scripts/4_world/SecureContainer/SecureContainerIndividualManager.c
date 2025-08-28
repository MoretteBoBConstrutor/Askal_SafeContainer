class SecureContainerIndividualManager
{
    private static ref SecureContainerIndividualManager m_Instance;
    private string m_PlayersContainerPath = "$profile:SafeContainer/PlayersContainer/";
    
    void SecureContainerIndividualManager()
    {
        // Garante que o diretório para arquivos individuais existe
        if (!FileExist(m_PlayersContainerPath))
        {
            MakeDirectory(m_PlayersContainerPath);
            Print("[SecureContainerIndividualManager] Diretório PlayersContainer criado: " + m_PlayersContainerPath);
        }
    }
    
    static SecureContainerIndividualManager GetInstance()
    {
        if (!m_Instance)
        {
            m_Instance = new SecureContainerIndividualManager();
        }
        return m_Instance;
    }
    
    // Salva o conteúdo do container seguro antes da morte (MESMA LÓGICA DO ORIGINAL)
    void SaveSecureContainerContents(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
        
        // Verifica se o sistema está habilitado
        SecureContainerConfig config = SecureContainerLevels.GetSecureContainerConfig();
        if (!config.EnableSecureContainer || !config.RestoreOnDeath)
        {
            //Print("[SecureContainerIndividualManager] Sistema desabilitado na configuração");
            return;
        }
            
        string playerUID = player.GetIdentity().GetPlainId();
        
        // Usa a função do PlayerBase para buscar o container no slot
        Askal_SecureContainer secureContainer = player.GetSecureContainer();
        if (!secureContainer)
        {
            //Print("[SecureContainerIndividualManager] Nenhum container seguro encontrado no attachment para: " + playerUID);
            return;
        }
            
        // Serializa todos os itens do container
        SecureContainerData containerData = new SecureContainerData(playerUID);
        SerializeContainerContents(secureContainer, containerData);
        
        // Só salva se houver itens
        if (containerData.StoredItems.Count() == 0)
        {
            //Print("[SecureContainerIndividualManager] Container vazio para: " + playerUID + " - nada para salvar");
            return;
        }
        
        // Salva no arquivo individual ao invés do BunkerDatabase
        SaveContainerDataToFile(playerUID, containerData);
        
        //Print("[SecureContainerIndividualManager] Conteúdo salvo para jogador: " + playerUID + " (" + containerData.StoredItems.Count() + " itens)");
    }
    
    // Restaura o conteúdo do container seguro após o respawn (MESMA LÓGICA DO ORIGINAL)
    void RestoreSecureContainerContents(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
        
        // Verifica se o sistema está habilitado
        SecureContainerConfig config = SecureContainerLevels.GetSecureContainerConfig();
        if (!config.EnableSecureContainer || !config.RestoreOnDeath)
        {
            //Print("[SecureContainerIndividualManager] Restauração desabilitada na configuração");
            return;
        }
            
        string playerUID = player.GetIdentity().GetPlainId();
        
        // Verifica se há arquivo individual salvo
        string playerFilePath = m_PlayersContainerPath + playerUID + ".json";
        if (!FileExist(playerFilePath))
        {
            //Print("[SecureContainerIndividualManager] Nenhum arquivo salvo encontrado para: " + playerUID);
            return;
        }
        
        // Carrega dados do arquivo individual ao invés do BunkerDatabase  
        SecureContainerData containerData = LoadContainerDataFromFile(playerUID);
        if (!containerData)
        {
            //Print("[SecureContainerIndividualManager] Dados corrompidos para: " + playerUID);
            return;
        }
        
        // Busca o container atual no attachment
        Askal_SecureContainer currentContainer = player.GetSecureContainer();
        if (!currentContainer)
        {
            //Print("[SecureContainerIndividualManager] ERRO: Container não encontrado no attachment para restauração: " + playerUID);
            return;
        }
            
        // Restaura todos os itens salvos no container existente
        RestoreItemsToContainer(currentContainer, containerData);
        
        // Remove arquivo após restauração (igual ao original que remove do database)
        DeleteFile(playerFilePath);
        
        Print("[SecureContainerIndividualManager] Conteúdo restaurado para jogador: " + playerUID + " (" + containerData.StoredItems.Count() + " itens)");
    }
    
    // FUNÇÕES AUXILIARES PARA ARQUIVO JSON (MAIS SEGURO QUE TXT)
    
    private void SaveContainerDataToFile(string playerUID, SecureContainerData containerData)
    {
        string playerFilePath = m_PlayersContainerPath + playerUID + ".json";
        
        // Usa JsonFileLoader para serialização segura
        JsonFileLoader<SecureContainerData>.JsonSaveFile(playerFilePath, containerData);
        //Print("[SecureContainerIndividualManager] Dados salvos em JSON: " + playerFilePath);
    }
    
    private SecureContainerData LoadContainerDataFromFile(string playerUID)
    {
        string playerFilePath = m_PlayersContainerPath + playerUID + ".json";
        if (!FileExist(playerFilePath))
            return null;
        
        // Cria instância vazia para carregar dados
        SecureContainerData containerData = new SecureContainerData(playerUID);
        
        // Usa JsonFileLoader para deserialização segura
        JsonFileLoader<SecureContainerData>.JsonLoadFile(playerFilePath, containerData);
        
        if (!containerData.StoredItems || containerData.StoredItems.Count() == 0)
        {
            //Print("[SecureContainerIndividualManager] Nenhum item salvo no arquivo: " + playerFilePath);
            return null;
        }
            
        return containerData;
    }
    
    // FUNÇÕES AUXILIARES (COPIADAS EXATAMENTE DO SECURECONTAINERMANAGER ANTIGO)
    
    private void SerializeContainerContents(Askal_SecureContainer container, SecureContainerData data)
    {
        CargoBase cargo = container.GetInventory().GetCargo();
        if (!cargo)
        {
            //Print("[SecureContainerIndividualManager] Container não possui cargo - possível erro");
            return;
        }
            
        int itemCount = cargo.GetItemCount();
        //Print("[SecureContainerIndividualManager] Serializando " + itemCount + " itens do container");
        
        for (int i = 0; i < itemCount; i++)
        {
            EntityAI item = cargo.GetItem(i);
            if (!item)
                continue;
                
            SecureItemData itemData = new SecureItemData();
            
            // Dados básicos
            itemData.ClassName = item.GetType();
            itemData.Health = item.GetHealth01("", "");
            
            // Posição no inventário
            int row, col, width, height;
            cargo.GetItemRowCol(i, row, col);
            cargo.GetItemSize(i, width, height);
            itemData.Row = row;
            itemData.Col = col;
            itemData.IsFlipped = item.GetInventory().GetFlipCargo();
            
            // Quantidade para itens que têm
            ItemBase itemBase = ItemBase.Cast(item);
            if (itemBase && itemBase.HasQuantity())
            {
                itemData.Quantity = itemBase.GetQuantity();
            }
            
            // Munição para magazines
            Magazine_Base magazine = Magazine_Base.Cast(item);
            if (magazine)
            {
                itemData.HasAmmo = true;
                itemData.AmmoCount = magazine.GetAmmoCount();
            }
            
            // Estado da comida
            Edible_Base food = Edible_Base.Cast(item);
            if (food)
            {
                itemData.IsFood = true;
                itemData.FoodStageValue = food.GetFoodStageType();
            }

            // 🌡️ TEMPERATURA - SALVAMENTO
            if (item.CanHaveTemperature())
            {
                itemData.HasTemperature = true;
                itemData.Temperature = item.GetTemperature();
                Print("[TEMPERATURA] Item " + itemData.ClassName + " salvo com temp: " + itemData.Temperature + "°C");
            }

            //✅NOVA FUNÇÃO: Serializa itens anexados e Itens dentro de inventarios
            SerializeItemRecursive(item, itemData);
            //
            data.StoredItems.Insert(itemData);
            //Print("[SecureContainerIndividualManager] Item serializado: " + itemData.ClassName + " (Pos: " + row + "," + col + ")");
        }

        // ✅ Serializar attachments diretos do container (VestPouch, etc.)
        if (container.GetInventory().AttachmentCount() > 0)
        {
            // Print("[SecureContainerIndividualManager] Serializando " + container.GetInventory().AttachmentCount() + " attachments do container");

            for (int j = 0; j < container.GetInventory().AttachmentCount(); j++)
            {
                EntityAI containerAttachment = container.GetInventory().GetAttachmentFromIndex(j);
                if (!containerAttachment)
                    continue;

                SecureItemData containerAttachData = new SecureItemData();

                // Dados básicos do attachment do container
                containerAttachData.ClassName = containerAttachment.GetType();
                containerAttachData.Health = containerAttachment.GetHealth01("", "");

                // Flag especial para identificar attachments do container
                containerAttachData.IsContainerAttachment = true;
                containerAttachData.Row = -1; // Flag especial para attachments do container
                containerAttachData.Col = j;  // Índice do attachment no container
                containerAttachData.IsFlipped = false;

                // Serializar propriedades básicas do attachment
                SerializeItemProperties(containerAttachment, containerAttachData);

                // ✅ IMPORTANTE: Serializar recursivamente o conteúdo do attachment
                SerializeItemRecursive(containerAttachment, containerAttachData);

                data.StoredItems.Insert(containerAttachData);
                // Print("[SecureContainerIndividualManager] Attachment do container serializado: " + containerAttachData.ClassName);
            }
        }
    }
    
    private void RestoreItemsToContainer(Askal_SecureContainer container, SecureContainerData data)
    {
        //Print("[SecureContainerIndividualManager] Restaurando " + data.StoredItems.Count() + " itens para o container");
        
        // Limpa o container antes de restaurar (caso tenha algo)
        CargoBase cargo = container.GetInventory().GetCargo();
        if (cargo)
        {
            // Remove todos os itens existentes um por um
            for (int i = cargo.GetItemCount() - 1; i >= 0; i--)
            {
                EntityAI item = cargo.GetItem(i);
                if (item)
                {
                    item.Delete();
                }
            }
        }
        
        foreach (SecureItemData itemData : data.StoredItems)
        {
            // ✅ VERIFICAR SE É ATTACHMENT DO CONTAINER OU ITEM DO CARGO
            if (itemData.IsContainerAttachment)
            {
                // ✅ PULAR attachments do container neste loop - serão processados depois
                continue;
            }

            // Cria o item no container na posição exata
            EntityAI restoredItem = container.GetInventory().CreateEntityInCargoEx( itemData.ClassName, container.GetInventory().GetCargo().GetOwnerCargoIndex(), itemData.Row, itemData.Col, itemData.IsFlipped);
            
            if (!restoredItem)
            {
                //Print("[SecureContainerIndividualManager] ERRO: Falha ao restaurar item: " + itemData.ClassName + " na posição " + itemData.Row + "," + itemData.Col);
                continue;
            }
                
            // Restaura propriedades do item
            restoredItem.SetHealth01("", "", itemData.Health);
            
            // Quantidade
            ItemBase itemBase = ItemBase.Cast(restoredItem);
            if (itemBase && itemBase.HasQuantity() && itemData.Quantity > 0)
            {
                itemBase.SetQuantity(itemData.Quantity);
            }
            
            // Munição
            if (itemData.HasAmmo)
            {
                Magazine_Base magazine = Magazine_Base.Cast(restoredItem);
                if (magazine)
                {
                    magazine.ServerSetAmmoCount(itemData.AmmoCount);
                }
            }
            
            // Estado da comida
            if (itemData.IsFood)
            {
                Edible_Base food = Edible_Base.Cast(restoredItem);
                if (food)
                {
                    food.ChangeFoodStage(itemData.FoodStageValue);
                }
            }

            // 🌡️ TEMPERATURA - RESTAURAÇÃO
            if (itemData.HasTemperature && restoredItem.CanHaveTemperature())
            {
                restoredItem.SetTemperatureDirect(itemData.Temperature);
                Print("[TEMPERATURA] Item " + itemData.ClassName + " restaurado com temp: " + itemData.Temperature + "°C");
            }

            //✅NOVA FUNÇÃO: Restaura itens anexados e Itens dentro de inventarios
            RestoreItemRecursive(restoredItem, itemData);
            //
            //Print("[SecureContainerIndividualManager] Item restaurado: " + itemData.ClassName + " (Pos: " + itemData.Row + "," + itemData.Col + ")");
        }
        
        //✅ Restaurar attachments diretos do container
        foreach (SecureItemData itemData2 : data.StoredItems)
        {
            if (!itemData2.IsContainerAttachment)
            {
                // Pular itens que não são attachments do container
                continue;
            }
            
            //Print("[SecureContainerIndividualManager] Restaurando attachment do container: " + itemData.ClassName);
            
            // Criar attachment diretamente no container
            EntityAI restoredAttachment = container.GetInventory().CreateInInventory(itemData2.ClassName);
            
            if (!restoredAttachment)
            {
                //Print("[SecureContainerIndividualManager] ERRO: Falha ao restaurar attachment do container: " + itemData.ClassName);
                continue;
            }
            
            // Restaurar propriedades do attachment
            restoredAttachment.SetHealth01("", "", itemData2.Health);
            
            // Restaurar propriedades específicas do attachment
            RestoreItemProperties(restoredAttachment, itemData2);
            
            // ✅ IMPORTANTE: Restaurar recursivamente o conteúdo do attachment
            RestoreItemRecursive(restoredAttachment, itemData2);
            
            //Print("[SecureContainerIndividualManager] Attachment do container restaurado: " + itemData.ClassName);
        }


        //Print("[SecureContainerIndividualManager] Restauração completa - " + data.StoredItems.Count() + " itens processados");
    }
    
    // Limpa dados de um jogador (deleta arquivo, igual ao original)
    void ClearPlayerData(string steamID64)
    {
        if (steamID64 == "") return;
        
        string playerFilePath = m_PlayersContainerPath + steamID64 + ".json";
        if (FileExist(playerFilePath))
        {
            DeleteFile(playerFilePath);
            //Print("[SecureContainerIndividualManager] Dados limpos para: " + steamID64);
        }
    }
    
    // Verifica se tem dados salvos
    bool HasPlayerData(string steamID64)
    {
        if (steamID64 == "") return false;
        string playerFilePath = m_PlayersContainerPath + steamID64 + ".json";
        
        // Verifica se arquivo existe e tem conteúdo
        if (!FileExist(playerFilePath)) return false;
        
        // Carrega para verificar se tem itens
        SecureContainerData data = LoadContainerDataFromFile(steamID64);
        return (data && data.StoredItems && data.StoredItems.Count() > 0);
    }

    // ✅ FUNÇÃO COMPLETAMENTE NOVA
    private void SerializeItemRecursive(EntityAI item, SecureItemData itemData)
    {
        // Verificar se item tem inventário (ProtectorCase, FirstAidKit)
        if (item.GetInventory().GetCargo())
        {
            itemData.HasInventory = true;
            itemData.NestedItems = new array<ref SecureItemData>();

            // Serializar itens dentro deste item
            CargoBase itemCargo = item.GetInventory().GetCargo();
            int nestedCount = itemCargo.GetItemCount();

            for (int j = 0; j < nestedCount; j++)
            {
                EntityAI nestedItem = itemCargo.GetItem(j);
                if (!nestedItem)
                    continue;

                SecureItemData nestedData = new SecureItemData();

                // Dados básicos do item aninhado
                nestedData.ClassName = nestedItem.GetType();
                nestedData.Health = nestedItem.GetHealth01("", "");

                // Posição dentro do item pai
                int nRow, nCol;
                itemCargo.GetItemRowCol(j, nRow, nCol);
                nestedData.Row = nRow;
                nestedData.Col = nCol;
                nestedData.IsFlipped = nestedItem.GetInventory().GetFlipCargo();

                // Propriedades específicas (quantidade, munição, comida)
                SerializeItemProperties(nestedItem, nestedData);

                itemData.NestedItems.Insert(nestedData);
            }
        }

        // Serializar attachments
        if (item.GetInventory().AttachmentCount() > 0)
        {
            itemData.Attachments = new array<ref SecureItemData>();

            for (int k = 0; k < item.GetInventory().AttachmentCount(); k++)
            {
                EntityAI attachment = item.GetInventory().GetAttachmentFromIndex(k);
                if (!attachment)
                    continue;

                SecureItemData attachData = new SecureItemData();
                attachData.ClassName = attachment.GetType();
                attachData.Health = attachment.GetHealth01("", "");

                SerializeItemProperties(attachment, attachData);

                itemData.Attachments.Insert(attachData);
            }
        }
    }

    // ✅ FUNÇÃO COMPLETAMENTE NOVA
    private void RestoreItemRecursive(EntityAI parentItem, SecureItemData itemData)
    {
        // Restaurar itens aninhados
        if (itemData.HasInventory && itemData.NestedItems && itemData.NestedItems.Count() > 0)
        {
            foreach (SecureItemData nestedData : itemData.NestedItems)
            {
                EntityAI nestedItem = parentItem.GetInventory().CreateEntityInCargoEx(nestedData.ClassName,parentItem.GetInventory().GetCargo().GetOwnerCargoIndex(),nestedData.Row,nestedData.Col,nestedData.IsFlipped);

                if (nestedItem)
                {
                    RestoreItemProperties(nestedItem, nestedData);
                }
            }
        }

        // Restaurar attachments
        if (itemData.Attachments && itemData.Attachments.Count() > 0)
        {
            foreach (SecureItemData attachData : itemData.Attachments)
            {
                EntityAI attachment = parentItem.GetInventory().CreateInInventory(attachData.ClassName);
                if (attachment)
                {
                    RestoreItemProperties(attachment, attachData);
                }
            }
        }
    }

    // ✅ FUNÇÃO AUXILIAR NOVA
    private void SerializeItemProperties(EntityAI item, SecureItemData itemData)
    {
        // Quantidade para itens que têm
        ItemBase itemBase = ItemBase.Cast(item);
        if (itemBase && itemBase.HasQuantity())
        {
            itemData.Quantity = itemBase.GetQuantity();
        }

        // Munição para magazines
        Magazine_Base magazine = Magazine_Base.Cast(item);
        if (magazine)
        {
            itemData.HasAmmo = true;
            itemData.AmmoCount = magazine.GetAmmoCount();
        }

        // Estado da comida
        Edible_Base food = Edible_Base.Cast(item);
        if (food)
        {
            itemData.IsFood = true;
            itemData.FoodStageValue = food.GetFoodStageType();
        }

        // 🌡️ TEMPERATURA - SALVAMENTO RECURSIVO
        if (item.CanHaveTemperature())
        {
            itemData.HasTemperature = true;
            itemData.Temperature = item.GetTemperature();
        }
    }

    // ✅ FUNÇÃO AUXILIAR NOVA
    private void RestoreItemProperties(EntityAI item, SecureItemData itemData)
    {
        // Restaurar saúde
        item.SetHealth01("", "", itemData.Health);

        // Quantidade
        ItemBase itemBase = ItemBase.Cast(item);
        if (itemBase && itemBase.HasQuantity() && itemData.Quantity > 0)
        {
            itemBase.SetQuantity(itemData.Quantity);
        }

        // Munição
        if (itemData.HasAmmo)
        {
            Magazine_Base magazine = Magazine_Base.Cast(item);
            if (magazine)
            {
                magazine.ServerSetAmmoCount(itemData.AmmoCount);
            }
        }

        // Estado da comida
        if (itemData.IsFood)
        {
            Edible_Base food = Edible_Base.Cast(item);
            if (food)
            {
                food.ChangeFoodStage(itemData.FoodStageValue);
            }
        }

        // 🌡️ TEMPERATURA - RESTAURAÇÃO RECURSIVA
        if (itemData.HasTemperature && item.CanHaveTemperature())
        {
            item.SetTemperatureDirect(itemData.Temperature);
        }
    }
}
