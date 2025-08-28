class Askal_SecureContainer : Container_Base
{
    // UID do proprietário deste container (para segurança) - apenas servidor
    private string m_OwnerUID = "";
    
    // Sincronização cliente-servidor do SteamID64 (igual DogTag_Base)
    int m_OwnerID_Part1; // Primeiros 8 dígitos
    int m_OwnerID_Part2; // Últimos 9 dígitos
    
    // Cache local do ID reconstruído para acesso rápido
    private string m_CachedOwnerID = "";
    
    void Askal_SecureContainer()
    {
        // Container seguro configurado para attachment
        SetTakeable(false); // Não pode ser pego diretamente
        
        // Inicializa as variáveis
        m_OwnerID_Part1 = 0;
        m_OwnerID_Part2 = 0;
        m_CachedOwnerID = "";
        
        // Registra as variáveis para sincronização cliente-servidor
        RegisterNetSyncVariableInt("m_OwnerID_Part1");
        RegisterNetSyncVariableInt("m_OwnerID_Part2");
        
        //Print("[SecureContainer] Container seguro inicializado para attachment");
    }
    
    // Define o proprietário deste container (chamado no spawn)
    void SetOwner(string uid)
    {
        m_OwnerUID = uid;
        
        // Divide o SteamID64 em duas partes para sincronização (igual DogTag_Base)
        if (uid == "" || uid.Length() != 17) 
        {
            m_OwnerID_Part1 = 0;
            m_OwnerID_Part2 = 0;
        }
        else
        {
            // Divide o ID em duas partes para armazenamento
            m_OwnerID_Part1 = uid.Substring(0, 8).ToInt();
            m_OwnerID_Part2 = uid.Substring(8, 9).ToInt();
        }
        
        // Reconstrói o cache imediatamente
        RebuildCachedID();
        
        // Marca para sincronização com clientes
        SetSynchDirty();
        
        //Print("[SecureContainer] Container proprietário definido: " + uid);
    }
    
    // Retorna o proprietário do container (apenas para debug/logs)
    string GetOwnerUID()
    {
        return m_OwnerUID;
    }
    
    // Chamado quando variáveis sincronizadas são atualizadas
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        RebuildCachedID();
    }
    
    // Função para reconstruir o ID em cache a partir das partes (igual DogTag_Base)
    private void RebuildCachedID()
    {
        if (m_OwnerID_Part1 == 0 && m_OwnerID_Part2 == 0)
        {
            m_CachedOwnerID = "";
        }
        else
        {
            string part1_str = string.Format("%1", m_OwnerID_Part1);
            string part2_str = string.Format("%1", m_OwnerID_Part2);

            while (part2_str.Length() < 9)
            {
                part2_str = "0" + part2_str;
            }
            m_CachedOwnerID = part1_str + part2_str;
        }
    }
    
    // Retorna o SteamID64 sincronizado (funciona no cliente)
    string GetSynchedOwnerID()
    {
        return m_CachedOwnerID;
    }
    
    override bool CanPutIntoHands(EntityAI parent)
    {
        return false; // Container seguro nunca vai para as mãos
    }
    
    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if (!super.CanReceiveItemIntoCargo(item))
        {
            //Print("[SecureContainer] DEBUG: super.CanReceiveItemIntoCargo = false para: " + item.GetType());
            return false;
        }
        
        // DEBUG: Log básico
        //Print("[SecureContainer] DEBUG: Tentativa de adicionar item: " + item.GetType());
        //Print("[SecureContainer] DEBUG: Container Owner ID Sincronizado: " + GetSynchedOwnerID());
        
        // RESTRIÇÃO DE ITENS - Verifica recursivamente se o item ou seus conteúdos são permitidos
        if (!IsItemAllowedRecursive(item))
        {
            //Print("[SecureContainer] Item REJEITADO - contém itens não permitidos: " + item.GetType());
            return false;
        }
        
        // VERIFICAÇÃO DE PROPRIETÁRIO usando ID sincronizado (funciona client-side)
        PlayerBase owner = PlayerBase.Cast(GetHierarchyParent());
        if (owner && IsAttachedToPlayer(owner))
        {
            // BLOQUEAR se o proprietário está amarrado/inconsciente
            if (owner.IsUnconscious() || owner.IsRestrained())
            {
                return false;
            }
            // Usa o ID sincronizado que funciona no cliente
            string currentUID = "";
            if (owner.GetIdentity())
            {
                currentUID = owner.GetIdentity().GetPlainId();
            }
            
            string containerOwnerID = GetSynchedOwnerID();
            //Print("[SecureContainer] DEBUG: Current Player UID: " + currentUID);
            //Print("[SecureContainer] DEBUG: Container Owner UID (Synced): " + containerOwnerID);
            
            if (currentUID != containerOwnerID || containerOwnerID == "")
            {
                //Print("[SecureContainer] DEBUG: UIDs NÃO COMBINAM ou container sem dono! Bloqueando acesso.");
                return false;
            }
        }
        
        //Print("[SecureContainer] DEBUG: Item PERMITIDO: " + itemClassName);
        return true;
    }
    
    // Override para bloquear remoção de itens por outros jogadores
    override bool CanReleaseCargo(EntityAI cargo)
    {
        if (!super.CanReleaseCargo(cargo))
            return false;
            
        //Print("[SecureContainer] DEBUG: Tentativa de remoção de item: " + cargo.GetType());
        
        // VERIFICAÇÃO DE PROPRIETÁRIO usando ID sincronizado (funciona client-side)
        PlayerBase owner = PlayerBase.Cast(GetHierarchyParent());
        if (owner && IsAttachedToPlayer(owner))
        {
            if (owner.IsUnconscious() || owner.IsRestrained())
            {
                return false;
            }
            // Usa o ID sincronizado que funciona no cliente
            string currentUID = "";
            if (owner.GetIdentity())
            {
                currentUID = owner.GetIdentity().GetPlainId();
            }
            
            string containerOwnerID = GetSynchedOwnerID();
            //Print("[SecureContainer] DEBUG: Current Player UID: " + currentUID);
            //Print("[SecureContainer] DEBUG: Container Owner UID (Synced): " + containerOwnerID);
            
            if (currentUID != containerOwnerID || containerOwnerID == "")
            {
                //Print("[SecureContainer] DEBUG: UIDs NÃO COMBINAM ou container sem dono! Bloqueando remoção.");
                return false;
            }
        }
        
        //Print("[SecureContainer] DEBUG: Remoção PERMITIDA: " + cargo.GetType());
        return true;
    }

    // ✅ NOVA FUNÇÃO: Verifica se attachments podem ser anexados ao container
    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (!super.CanReceiveAttachment(attachment, slotId))
            return false;
            
        // Verificar proprietário (mesma lógica do cargo)
        PlayerBase owner = PlayerBase.Cast(GetHierarchyParent());
        if (owner && IsAttachedToPlayer(owner))
        {
            if (owner.IsUnconscious() || owner.IsRestrained())
            {
                return false;
            }
            
            string currentUID = "";
            if (owner.GetIdentity())
            {
                currentUID = owner.GetIdentity().GetPlainId();
            }
            
            string containerOwnerID = GetSynchedOwnerID();
            if (currentUID != containerOwnerID || containerOwnerID == "")
            {
                return false;
            }
        }
        
        // ✅ VERIFICAÇÃO RECURSIVA DE RESTRIÇÕES PARA ATTACHMENTS
        if (!IsItemAllowedRecursive(attachment))
        {
            //Print("[SecureContainer] Attachment REJEITADO - contém itens não permitidos: " + attachment.GetType());
            return false;
        }
        
        return true;
    }

    override bool CanDisplayCargo()
    {
        if (!super.CanDisplayCargo())
            return false;

        PlayerBase owner = PlayerBase.Cast(GetHierarchyParent());
        if (owner && IsAttachedToPlayer(owner))
        {
            // ESCONDE inventário se proprietário amarrado/inconsciente
            if (owner.IsUnconscious() || owner.IsRestrained())
            {
                return false;
            }
        }

        return true;
    }

    // Verifica se este container está anexado ao jogador especificado
    private bool IsAttachedToPlayer(PlayerBase player)
    {
        if (!player)
            return false;
            
        // Verifica se está no slot StorageSlot do jogador
        EntityAI attachedItem = player.FindAttachmentBySlotName("StorageSlot");
        return (attachedItem == this);
    }
    
    // Override para debug quando o container é destruído
    override void EEDelete(EntityAI parent)
    {
        //Print("[SecureContainer] Container sendo deletado - proprietário: " + m_OwnerUID);
        super.EEDelete(parent);
    }
    
    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        
        // Salva as partes INT (igual DogTag_Base)
        ctx.Write(m_OwnerID_Part1);
        ctx.Write(m_OwnerID_Part2);
        
        // Também salva a string como backup
        ctx.Write(m_OwnerUID);
    }
    
    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
            
        // Carrega as partes INT (igual DogTag_Base)
        if (!ctx.Read(m_OwnerID_Part1))
            m_OwnerID_Part1 = 0;
            
        if (!ctx.Read(m_OwnerID_Part2))
            m_OwnerID_Part2 = 0;
            
        // Carrega a string como backup
        if (!ctx.Read(m_OwnerUID))
            m_OwnerUID = "";
            
        // Reconstrói o cache (igual DogTag_Base)
        RebuildCachedID();
            
        //Print("[SecureContainer] Container carregado com proprietário: " + m_OwnerUID);
        return true;
    }
    
    // Adiciona AfterStoreLoad (igual DogTag_Base)
    override void AfterStoreLoad()
    {
        super.AfterStoreLoad();
        
        // Força sincronização após carregar (igual DogTag_Base)
        SetSynchDirty();
    }

    // ✅ NOVA FUNÇÃO: Verifica recursivamente se um item ou seus conteúdos são restritos
    bool IsItemAllowedRecursive(EntityAI item)
    {
        if (!item)
            return true;
            
        SecureContainerConfig config = SecureContainerLevels.GetSecureContainerConfig();
        
        // Verifica o item principal
        if (!config.IsItemAllowed(item.GetType()))
        {
            return false;
        }
        
        // Verifica itens no inventário (cargo) do item
        if (item.GetInventory().GetCargo())
        {
            CargoBase itemCargo = item.GetInventory().GetCargo();
            int cargoCount = itemCargo.GetItemCount();
            
            for (int i = 0; i < cargoCount; i++)
            {
                EntityAI cargoItem = itemCargo.GetItem(i);
                if (cargoItem)
                {
                    // Verificação recursiva para itens dentro do cargo
                    if (!IsItemAllowedRecursive(cargoItem))
                    {
                        return false;
                    }
                }
            }
        }
        
        // Verifica attachments do item
        if (item.GetInventory().AttachmentCount() > 0)
        {
            for (int j = 0; j < item.GetInventory().AttachmentCount(); j++)
            {
                EntityAI attachment = item.GetInventory().GetAttachmentFromIndex(j);
                if (attachment)
                {
                    // Verificação recursiva para attachments
                    if (!IsItemAllowedRecursive(attachment))
                    {
                        return false;
                    }
                }
            }
        }
        
        return true; // Permitido se chegou até aqui
    }
}
