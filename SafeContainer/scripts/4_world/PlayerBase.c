// Modded PlayerBase para registrar a action customizada de upgrades

modded class PlayerBase
{
    // Função para buscar o Secure Container no slot dedicado
    Askal_SecureContainer GetSecureContainer()
    {
        EntityAI attachment = FindAttachmentBySlotName("StorageSlot");
        if (attachment)
        {
            return Askal_SecureContainer.Cast(attachment);
        }
        return null;
    }
    
    // Cria e anexa um Secure Container ao jogador
    void GivePlayerSecureContainer()
    {
        // Só cria se não existe um
        if (GetSecureContainer())
            return;
        
        // SISTEMA PREMIUM: Obtém a classe correta baseada no nível do player
        string playerUID = GetIdentity().GetPlainId();
        string containerClass = SecureContainerLevels.GetPlayerContainerClassName(playerUID);
        int playerLevel = SecureContainerLevels.GetPlayerContainerLevel(playerUID);
        
        //Print("[SecureContainer] Criando container nível " + playerLevel + " (" + containerClass + ") para: " + GetIdentity().GetName());
            
        Askal_SecureContainer container = Askal_SecureContainer.Cast(GetInventory().CreateAttachment(containerClass));
        if (container && GetIdentity())
        {
            container.SetOwner(playerUID);
            //Print("[SecureContainer] Container nível " + playerLevel + " anexado ao jogador: " + GetIdentity().GetName());
        }
        else 
        {
            //Print("[SecureContainer] ERRO: Falha ao criar container nível " + playerLevel + " para: " + GetIdentity().GetName());
        }
    }
    
    // Bloqueia a remoção do Secure Container (igual às DogTags)
    override bool CanReleaseAttachment(EntityAI attachment)
    {
        // Verifica se o anexo é um Secure Container
        if (attachment.IsInherited(Askal_SecureContainer))
        {
            return false;
        }
        return super.CanReleaseAttachment(attachment);
    }
    
    // Hook para salvar conteúdo do secure container na morte
    override void EEKilled(Object killer)
    {
        // Salvar conteúdo do secure container antes da morte
        if (GetGame().IsServer())
        {
            // SISTEMA INDEPENDENTE: Salva usando a mesma lógica do sistema original
            SecureContainerIndividualManager.GetInstance().SaveSecureContainerContents(this);
            //Print("[SecureContainer] Conteúdo salvo em arquivo individual para: " + GetIdentity().GetName());

            // REMOVER o container do corpo para que fique invisível
            Askal_SecureContainer container = GetSecureContainer();
            if (container)
            {
                //Print("[SecureContainer] Removendo container do corpo morto de: " + GetIdentity().GetName());
                container.Delete();
            }
        }
        
        super.EEKilled(killer);
    }
    
    // Hook para quando jogador é completamente carregado
    override void OnPlayerLoaded()
    {
        super.OnPlayerLoaded();
        
        // Só executar no servidor
        if (!GetGame().IsServer())
            return;
            
        // Dar secure container para jogador (após um delay para garantir que tudo foi carregado)
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.CheckAndGiveSecureContainer, 3000, false);
    }
    
    // Função privada para verificar e dar secure container
    private void CheckAndGiveSecureContainer()
    {
        // Só executar no servidor
        if (!GetGame().IsServer())
            return;
            
        GivePlayerSecureContainer();
        
        // SISTEMA INDEPENDENTE: Restaurar usando a mesma lógica do sistema original
        SecureContainerIndividualManager.GetInstance().RestoreSecureContainerContents(this);
        //Print("[SecureContainer] Conteúdo restaurado de arquivo individual para: " + GetIdentity().GetName());
    }
}
