// NOVAPLAYObjectiveComponent.cpp
#include "NOVAPLAYObjectiveComponent.h"
#include "NOVAPLAYInventoryComponent.h"

UNOVAPLAYObjectiveComponent::UNOVAPLAYObjectiveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNOVAPLAYObjectiveComponent::InitializeWithInventory(UNOVAPLAYInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent)
	{
		return;
	}

	// Si ya estabamos enganchados a otro inventario (o a este mismo), soltamos el binding anterior
	if (CachedInventory)
	{
		CachedInventory->OnInventoryChanged.RemoveDynamic(this, &UNOVAPLAYObjectiveComponent::HandleInventoryChanged);
	}

	CachedInventory = InInventoryComponent;

	// A partir de ahora, cada vez que se agrega/saca algo del inventario, revisamos si
	// eso afecta al objetivo activo (solo importa si ese objetivo usa contador).
	CachedInventory->OnInventoryChanged.AddUniqueDynamic(this, &UNOVAPLAYObjectiveComponent::HandleInventoryChanged);
}

void UNOVAPLAYObjectiveComponent::StartObjectiveChain(const FText& InMainObjectiveTitle, const TArray<FObjectiveDefinition>& InChain)
{
	// Lo guardamos para poder resincronizar al HUD si se suscribe después de este llamado
	// (ver GetMainObjectiveTitle y su uso en UNOVAPLAYHUDWidget::InitializeObjectives).
	MainObjectiveTitle = InMainObjectiveTitle;

	Chain = InChain;
	VisibleObjectives.Empty();
	CurrentIndex = INDEX_NONE;

	// Primero el cartel grande con el objetivo principal del nivel (ej "Escapa de la biblioteca").
	// El HUD se encarga de mostrarlo y desvanecerlo con su propia animación.
	OnMainObjectiveRequested.Broadcast(InMainObjectiveTitle);

	// Después arrancamos la cadena en el primer objetivo.
	ActivateObjective(0);
}

bool UNOVAPLAYObjectiveComponent::CompleteCurrentObjective()
{
	if (!Chain.IsValidIndex(CurrentIndex) || VisibleObjectives.Num() == 0)
	{
		return false;
	}

	if (VisibleObjectives.Last().bCompleted)
	{
		return false; // ya estaba completado, evitamos disparar el evento dos veces
	}

	CompleteCurrentObjectiveInternal();
	return true;
}

bool UNOVAPLAYObjectiveComponent::IsChainComplete() const
{
	return Chain.Num() > 0
		&& CurrentIndex == INDEX_NONE
		&& VisibleObjectives.Num() > 0
		&& VisibleObjectives.Last().bCompleted;
}

void UNOVAPLAYObjectiveComponent::HandleInventoryChanged()
{
	if (!Chain.IsValidIndex(CurrentIndex) || VisibleObjectives.Num() == 0)
	{
		return;
	}

	const FObjectiveDefinition& Def = Chain[CurrentIndex];
	if (!Def.bUseCounter)
	{
		return; // el objetivo activo no depende de items, no hay nada que recalcular
	}

	FObjectiveEntry& ActiveEntry = VisibleObjectives.Last();
	if (ActiveEntry.bCompleted)
	{
		return;
	}

	const int32 NewCount = CountTrackedItems(Def);
	if (NewCount == ActiveEntry.CurrentCount)
	{
		return; // nada cambió para este objetivo en particular
	}

	ActiveEntry.CurrentCount = NewCount;
	ActiveEntry.Description = BuildDisplayText(Def, NewCount);
	OnObjectivesChanged.Broadcast();

	if (Def.TargetCount > 0 && NewCount >= Def.TargetCount)
	{
		CompleteCurrentObjectiveInternal();
	}
}

void UNOVAPLAYObjectiveComponent::ActivateObjective(int32 Index)
{
	if (!Chain.IsValidIndex(Index))
	{
		// No hay más objetivos: la cadena terminó (por ejemplo, ya solo falta escapar
		// y eso se resuelve con el trigger de salida del nivel, no con un objetivo más).
		CurrentIndex = INDEX_NONE;
		OnObjectivesChanged.Broadcast();
		return;
	}

	CurrentIndex = Index;
	const FObjectiveDefinition& Def = Chain[Index];

	FObjectiveEntry Entry;
	Entry.ObjectiveID = Def.ObjectiveID;
	Entry.bCompleted = false;
	Entry.bUseCounter = Def.bUseCounter;
	Entry.TargetCount = Def.TargetCount;
	Entry.CurrentCount = Def.bUseCounter ? CountTrackedItems(Def) : 0;
	Entry.Description = BuildDisplayText(Def, Entry.CurrentCount);

	VisibleObjectives.Add(Entry);
	OnObjectivesChanged.Broadcast();

	// Por si el jugador ya cumplía la condición antes de que este objetivo se activara
	// (por ejemplo, ya tenía todos los libros de un objetivo de recolección anterior).
	if (Def.bUseCounter && Def.TargetCount > 0 && Entry.CurrentCount >= Def.TargetCount)
	{
		CompleteCurrentObjectiveInternal();
	}
}

void UNOVAPLAYObjectiveComponent::CompleteCurrentObjectiveInternal()
{
	if (VisibleObjectives.Num() == 0)
	{
		return;
	}

	FObjectiveEntry& ActiveEntry = VisibleObjectives.Last();
	if (ActiveEntry.bCompleted)
	{
		return;
	}

	ActiveEntry.bCompleted = true;

	// Primero el flash de "Objetivo completado" con el texto de lo que se acaba de cumplir...
	OnObjectiveCompleted.Broadcast(ActiveEntry.Description);
	// ...y avisamos también que la lista cambió (para que quede tildado en el HUD).
	OnObjectivesChanged.Broadcast();

	// Pasamos al siguiente objetivo de la cadena (si hay uno).
	ActivateObjective(CurrentIndex + 1);
}

int32 UNOVAPLAYObjectiveComponent::CountTrackedItems(const FObjectiveDefinition& Def) const
{
	if (!CachedInventory)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FName& ItemID : Def.TrackedItemIDs)
	{
		if (CachedInventory->HasItem(ItemID))
		{
			Count++;
		}
	}
	return Count;
}

FText UNOVAPLAYObjectiveComponent::BuildDisplayText(const FObjectiveDefinition& Def, int32 CurrentCount) const
{
	if (Def.bUseCounter)
	{
		return FText::Format(INVTEXT("{0} ({1}/{2})"), Def.Description, FText::AsNumber(CurrentCount), FText::AsNumber(Def.TargetCount));
	}
	return Def.Description;
}
