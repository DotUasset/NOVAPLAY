// NOVAPLAYObjectiveComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NOVAPLAYObjectiveComponent.generated.h"

class UNOVAPLAYInventoryComponent;

// Definición de UN objetivo dentro de la cadena de objetivos de un nivel.
// Esto lo configura el nivel (ver StartObjectiveChain), no el personaje: así cada
// nivel puede tener su propia cadena distinta (biblioteca, aula, etc.).
USTRUCT(BlueprintType)
struct FObjectiveDefinition
{
	GENERATED_BODY()

	// Identificador interno del objetivo (ej: "COLLECT_BOOKS"). Útil para debug/lógica futura.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveID;

	// Texto base a mostrar, ej "Obtener los libros". Si bUseCounter es true, el
	// componente le agrega automáticamente "(X/Y)" al final.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText Description;

	// Si es true, este objetivo se completa solo cuando el inventario del jugador
	// contiene todos los ítems listados en TrackedItemIDs (progreso automático).
	// Si es false, hay que completarlo a mano desde el nivel llamando a
	// CompleteCurrentObjective() (ej: al interactuar con la computadora, al escapar).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bUseCounter = false;

	// Cantidad total a alcanzar (ej: 5 libros). Solo se usa si bUseCounter es true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 TargetCount = 0;

	// Ítems del inventario a trackear (ej: 5 IDs de libros distintos). El progreso
	// es la cantidad de estos IDs de los que el jugador tiene al menos 1 unidad.
	// Solo se usa si bUseCounter es true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	TArray<FName> TrackedItemIDs;
};

// Estado runtime de un objetivo ya activado (lo que efectivamente lee/muestra el HUD).
USTRUCT(BlueprintType)
struct FObjectiveEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	FName ObjectiveID;

	// Texto ya formateado y listo para mostrar (con el contador incluido si corresponde).
	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	bool bCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	bool bUseCounter = false;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	int32 TargetCount = 0;
};

// Se dispara cada vez que cambia algo en la lista de objetivos (progreso, completado,
// objetivo nuevo activado). El HUD se suscribe una vez y en cada disparo vuelve a pedir
// GetObjectives() para reconstruir la lista visual, igual que hace el inventario.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectivesChanged);

// Se dispara UNA vez, justo en el momento en que un objetivo pasa a estar completado.
// Pensado para que el HUD muestre el texto "Objetivo completado" con su animación propia,
// separado de la actualización normal de la lista.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FText, CompletedObjectiveText);

// Se dispara UNA vez al arrancar el nivel (StartObjectiveChain), con el objetivo
// principal del nivel (ej: "Escapa de la biblioteca"). El HUD lo muestra grande y
// después lo desvanece, antes de mostrar la lista de objetivos.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainObjectiveRequested, FText, MainObjectiveText);

/**
 * Maneja la cadena de objetivos de un nivel: cuál está activo, su progreso, y cuándo
 * pasar al siguiente. Vive en el personaje (igual que el inventario), pero la cadena
 * en sí la define el NIVEL llamando a StartObjectiveChain en su BeginPlay (por ejemplo,
 * desde un actor "BP_LevelObjectives" colocado en el mapa, o desde el Level Blueprint).
 *
 * Así cada nivel (biblioteca, aula, etc.) puede tener objetivos totalmente distintos
 * sin tocar el personaje ni este componente.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NOVAPLAY_API UNOVAPLAYObjectiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNOVAPLAYObjectiveComponent();

	UPROPERTY(BlueprintAssignable, Category = "Objectives")
	FOnObjectivesChanged OnObjectivesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Objectives")
	FOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Objectives")
	FOnMainObjectiveRequested OnMainObjectiveRequested;

	// Lo llama el personaje UNA vez en su BeginPlay, pasándole SU propio inventario.
	// A partir de acá el componente escucha los cambios de inventario para actualizar
	// solo los objetivos que usan contador (bUseCounter).
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void InitializeWithInventory(UNOVAPLAYInventoryComponent* InInventoryComponent);

	// Lo llama el NIVEL (un actor colocado en el mapa, o el Level Blueprint) en su
	// BeginPlay: define la cadena completa de objetivos de este nivel y arranca
	// mostrando el objetivo principal + el primer objetivo de la lista.
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void StartObjectiveChain(const FText& InMainObjectiveTitle, const TArray<FObjectiveDefinition>& InChain);

	// Lo llama cualquier sistema del nivel (computadora, puerta de salida, palanca, etc.)
	// cuando se cumple el objetivo activo actual y ese objetivo NO se mide solo. Si el
	// objetivo activo ya se completó, o no hay ninguno activo, no hace nada y devuelve false.
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	bool CompleteCurrentObjective();

	// Lista completa a mostrar en el HUD: los ya completados + el activo actual, en orden.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objectives")
	const TArray<FObjectiveEntry>& GetObjectives() const { return VisibleObjectives; }

	// True cuando ya se completó el último objetivo de la cadena (el nivel está "listo para escapar").
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objectives")
	bool IsChainComplete() const;

	// Devuelve el título del objetivo principal ya guardado (vacío si StartObjectiveChain
	// todavía no se llamó). Lo usa el HUD para resincronizarse si se suscribió tarde
	// (mismo problema que soluciona HandleObjectivesChanged, pero para este evento).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objectives")
	FText GetMainObjectiveTitle() const { return MainObjectiveTitle; }

private:
	UPROPERTY()
	FText MainObjectiveTitle;

	UPROPERTY()
	TArray<FObjectiveDefinition> Chain;

	UPROPERTY()
	TArray<FObjectiveEntry> VisibleObjectives;

	// Índice del objetivo actualmente activo dentro de Chain. INDEX_NONE si no hay ninguno
	// activo (cadena vacía o ya terminada).
	int32 CurrentIndex = INDEX_NONE;

	UPROPERTY()
	UNOVAPLAYInventoryComponent* CachedInventory = nullptr;

	// Enganchado al delegate del inventario: recalcula el progreso del objetivo activo
	// si ese objetivo usa contador.
	UFUNCTION()
	void HandleInventoryChanged();

	// Activa el objetivo en Chain[Index] (lo agrega a VisibleObjectives y avisa al HUD).
	// Si Index no es válido, significa que la cadena terminó.
	void ActivateObjective(int32 Index);

	// Marca el objetivo activo como completado, avisa al HUD, y activa el siguiente.
	void CompleteCurrentObjectiveInternal();

	// Cuenta cuántos de los TrackedItemIDs de un objetivo ya están en el inventario.
	int32 CountTrackedItems(const FObjectiveDefinition& Def) const;

	// Arma el texto final a mostrar (con o sin contador) para un objetivo.
	FText BuildDisplayText(const FObjectiveDefinition& Def, int32 CurrentCount) const;
};
