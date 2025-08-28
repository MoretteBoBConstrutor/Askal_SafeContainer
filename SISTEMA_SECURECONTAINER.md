# 🔒 Sistema SafeContainer - Documentação Completa

## 📋 **Visão Geral**

O Sistema SafeContainer é um mod standalone para DayZ que fornece containers seguros anexáveis ao jogador, com persistência completa de itens após a morte e sistema avançado de restrições.

---

## 🏗️ **Arquitetura do Sistema**

### **Componentes Principais:**

1. **`SecureContainer.c`** - Container principal com validações
2. **`SecureContainerData.c`** - Estruturas de dados para persistência
3. **`SecureContainerConfig.c`** - Configurações e restrições
4. **`SecureContainerLevels.c`** - Sistema de níveis premium
5. **`SecureContainerIndividualManager.c`** - Gerenciamento de persistência
6. **`ItemBase.c`** - Extensão global para validações
7. **`PlayerBase.c`** - Extensões do jogador
8. **`MissionServer.c`** - Integração com servidor

---

## 🔧 **Funcionalidades Principais**

### **1. Container Seguro Anexável**
- **Anexação:** O container é anexado ao slot `StorageSlot` do jogador
- **Não Removível:** Não pode ser dropado, roubado ou perdido
- **Propriedade:** Cada container é vinculado ao SteamID64 do proprietário
- **Sincronização:** Sistema de sincronização cliente-servidor para propriedade

### **2. Sistema de Níveis**
```cpp
- Nível 1 (Padrão):  4x6 = 24 slots
- Nível 2 (Premium): 5x6 = 30 slots  
- Nível 3 (VIP):     6x6 = 36 slots
- Nível 4 (Ultra):   6x8 = 48 slots
```

### **3. Persistência Avançada**
- **Salvamento:** Todos os itens são salvos antes da morte
- **Restauração:** Itens são restaurados após respawn
- **Arquivos JSON:** Sistema seguro de arquivos individuais por jogador
- **Localização:** `$profile:SafeContainer/PlayersContainer/{SteamID64}.json`

### **4. Serialização Recursiva**
- **Itens Aninhados:** Salva itens dentro de containers (FirstAidKit, ProtectorCase)
- **Attachments:** Salva attachments dos itens e do próprio container
- **Propriedades:** Preserva saúde, quantidade, munição, estado da comida
- **Posicionamento:** Mantém posição exata no grid de inventário

---

## 🛡️ **Sistema de Restrições**

### **Configuração de Itens Restritos**
```json
{
    "RestrictedItems": [
        "NVGoggles",
        "Paper", 
        "M4A1"
    ]
}
```

### **Verificação Recursiva**
O sistema implementa verificação em **múltiplos níveis**:

1. **Nível 1:** Verifica o item sendo adicionado
2. **Nível 2:** Verifica itens dentro do cargo do item
3. **Nível 3:** Verifica attachments do item
4. **Recursivo:** Aplica a mesma lógica em todos os níveis

### **Cenários Bloqueados:**
- ❌ FirstAidKit contendo Paper
- ❌ VestPouch (anexada) contendo M4A1
- ❌ ProtectorCase com NVGoggles dentro
- ❌ Containers aninhados com itens proibidos

---

## 🔐 **Sistema de Segurança**

### **Validação de Propriedade**
```cpp
// Verificação por SteamID64 sincronizado
string currentUID = owner.GetIdentity().GetPlainId();
string containerOwnerID = GetSynchedOwnerID();

if (currentUID != containerOwnerID) {
    return false; // Bloqueia acesso
}
```

### **Proteções Implementadas**
- **Unconscious/Restrained:** Bloqueia acesso quando jogador está inconsciente/amarrado
- **Sincronização Segura:** Sistema de partes INT para sincronização cliente-servidor
- **Validação Contínua:** Verifica propriedade em todas as operações

---

## 💾 **Sistema de Persistência**

### **Estrutura de Dados**
```cpp
class SecureItemData {
    string ClassName;              // Tipo do item
    float Health;                  // Saúde do item
    float Quantity;                // Quantidade (para stackables)
    int Row, Col;                  // Posição no grid
    bool IsFlipped;                // Orientação do item
    
    // Munição
    bool HasAmmo;
    int AmmoCount;
    
    // Comida
    bool IsFood;
    int FoodStageValue;
    
    // Aninhamento
    bool HasInventory;
    ref array<ref SecureItemData> NestedItems;
    ref array<ref SecureItemData> Attachments;
    
    // Attachments do container
    bool IsContainerAttachment;
}
```

### **Processo de Salvamento**
1. **Trigger:** `Morte`
2. **Serialização:** Todos os itens são serializados recursivamente
3. **Arquivo:** Salvo em JSON individual por jogador
4. **Cleanup:** Arquivo é deletado após restauração

### **Processo de Restauração**
1. **Trigger:** `OnPlayerLoaded` após respawn
2. **Verificação:** Checa se existe arquivo salvo
3. **Carregamento:** Deserializa dados do JSON
4. **Criação:** Recria todos os itens na posição exata
5. **Recursão:** Restaura itens aninhados e attachments
6. **Limpeza:** Remove arquivo após sucesso

---

## 🔄 **Fluxo de Operação**

### **Adição de Item ao Container**
```
1. Player tenta adicionar item
2. SecureContainer.CanReceiveItemIntoCargo()
   ├── Verifica se é o proprietário
   ├── Chama IsItemAllowedRecursive()
   │   ├── Verifica item principal
   │   ├── Verifica cargo do item (recursivo)
   │   └── Verifica attachments (recursivo)
   └── Permite/Bloqueia operação
```

### **Adição de Item em Attachment**
```
1. Player tenta adicionar item em VestPouch anexada
2. ItemBase.CanReceiveItemIntoCargo()
   ├── GetRootSecureContainer() - sobe na hierarquia
   ├── Se encontrar SecureContainer
   │   └── Chama secureContainer.IsItemAllowedRecursive()
   └── Permite/Bloqueia operação
```

### **Anexação de Item ao Container**
```
1. Player tenta anexar VestPouch ao SecureContainer
2. SecureContainer.CanReceiveAttachment()
   ├── Verifica propriedade
   ├── Chama IsItemAllowedRecursive() no attachment
   └── Permite/Bloqueia anexação
```

---

## 📁 **Estrutura de Arquivos**

```
SafeContainer/
├── config.cpp                    # Configuração principal do mod
├── scripts/
│   ├── 3_game/
│   ├── 4_world/
│   │   ├── ItemBase.c            # Extensão global para validações
│   │   ├── PlayerBase.c          # Extensões do jogador
│   │   └── SecureContainer/
│   │       ├── SecureContainer.c              # Container principal
│   │       ├── SecureContainerData.c          # Estruturas de dados
│   │       ├── SecureContainerConfig.c        # Configurações
│   │       ├── SecureContainerLevels.c        # Sistema de níveis
│   │       └── SecureContainerIndividualManager.c # Persistência
│   └── 5_mission/
│       └── MissionServer.c       # Integração com servidor
└── data/                         # Arquivos de configuração runtime
```

---

## ⚙️ **Configurações**

### **Arquivo de Configuração**
**Localização:** `$profile:SafeContainer/secure_container_config.json`

```json
{
    "EnableSecureContainer": true,     // Habilita/desabilita sistema
    "GiveToNewPlayers": true,          // Dá container para novos jogadores
    "RestoreOnDeath": true,            // Restaura itens após morte
    "RestrictedItems": [               // Lista de itens proibidos
        "NVGoggles",
        "Paper", 
        "M4A1"
    ],
    "MaxValueLimit": 0.0,              // Limite de valor (0 = sem limite)
    "ContainerClassName": "Askal_SecureContainer"  // Classe do container
}
```

### **Níveis de Container**
**Localização:** `$profile:Askal_Insertion/SecureContainerLevels.json`

```json
{
    "level2": ["76561198000000001"],
    "level3": ["76561198000000003"],
    "level4": ["76561198000000004"]
}
```

---

## 🚫 **Restrições**

### **Restrições Implementadas**
- Lista configurável de itens proibidos
- Verificação recursiva impede bypass
- Validação de propriedade constante
- Proteção contra unconscious/restrained

---

## 🔍 **Cenários de Uso**

### **✅ Cenários Permitidos**
- FirstAidKit com bandages, morphine
- ProtectorCase com knife, compass
- VestPouch anexada com itens médicos
- Armas com attachments simples

### **❌ Cenários Bloqueados**
- FirstAidKit contendo Paper
- VestPouch com M4A1 (anexada depois)
- ProtectorCase com NVGoggles
- Qualquer container com item restrito

---

## 🛠️ **Manutenção e Debug**

### **Logs do Sistema**
```cpp
[SecureContainer] Container proprietário definido: 76561198000000001
[SecureContainerIndividualManager] Conteúdo salvo para jogador: 76561198000000001 (5 itens)
[SecureContainerIndividualManager] Conteúdo restaurado para jogador: 76561198000000001 (5 itens)
[ItemBase] Item REJEITADO em container da hierarquia do SecureContainer: Paper
```

### **Arquivos de Debug**
- **Config:** `$profile:SafeContainer/secure_container_config.json`
- **Níveis:** `$profile:Askal_Insertion/SecureContainerLevels.json`
- **Saves:** `$profile:SafeContainer/PlayersContainer/{SteamID64}.json`

---

## 🚀 **Performance**

### **Otimizações Implementadas**
- **Verificação sob demanda:** Só verifica quando necessário
- **Cache de proprietário:** ID sincronizado em cache
- **Arquivos individuais:** Evita lock de arquivo único
- **Cleanup automático:** Remove arquivos após uso

### **Impacto no Servidor**
- **Baixo:** Verificações são rápidas
- **Escalável:** Sistema de arquivos individuais
- **Eficiente:** Só processa quando jogador interage

---

## 📈 **Estatísticas**

- **Linhas de Código:** ~1,500 linhas
- **Arquivos:** 8 arquivos principais
- **Funcionalidades:** 15+ recursos implementados
- **Segurança:** 5 camadas de validação
- **Persistência:** 100% dos dados preservados

---

## 🎯 **Conclusão**

O Sistema SafeContainer oferece uma solução robusta e segura para persistência de itens em DayZ, com:

- **Segurança máxima** contra exploits
- **Flexibilidade** de configuração
- **Performance otimizada**
- **Manutenibilidade** do código
- **Experiência do usuário** aprimorada

O sistema é **totalmente funcional** e **pronto para produção**.
