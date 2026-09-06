// NOVAPLAY - BT Service: sincroniza la velocidad del docente (patrulla vs persecucion/
// investigacion) y le avisa a Blueprint (OnMovementStateChanged) que estado tiene ahora,
// para que el Event Graph de BP_Teacher decida sonido y mensajes de debug.

#include "BTService_UpdateMovementState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "TeacherCharacter.h"
#include "TeacherAIController.h"

UBTService_UpdateMovementState::UBTService_UpdateMovementState()
{
	NodeName = TEXT("Actualizar Velocidad Docente");

	// Corre seguido: es una comprobacion barata y necesitamos reaccionar rapido
	// apenas el docente ve o pierde al jugador.
	Interval = 0.15f;
	RandomDeviation = 0.03f;
}

void UBTService_UpdateMovementState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AICon || !BB)
	{
		return;
	}

	ATeacherCharacter* Teacher = Cast<ATeacherCharacter>(AICon->GetPawn());
	if (!Teacher)
	{
		return;
	}

	const bool bCanSee = BB->GetValueAsBool(ATeacherAIController::BBKey_CanSeePlayer);
	const bool bInvestigating = BB->IsVectorValueSet(ATeacherAIController::BBKey_LastKnownLocation);

	// Persigue a velocidad de chase si lo ve AHORA o si todavia va camino a la ultima
	// posicion conocida (investigando). Si no, vuelve a velocidad de patrulla.
	Teacher->SetMovementState(bCanSee || bInvestigating);

	// El estado especifico (mas detallado que el bool de arriba) se lo pasamos a Blueprint
	// para que decida sonido/debug. NotifyMovementState ya se encarga de no repetir el mismo
	// evento si el estado no cambio.
	const ETeacherMovementState NewState = bCanSee
		? ETeacherMovementState::Chasing
		: (bInvestigating ? ETeacherMovementState::Investigating : ETeacherMovementState::Patrolling);

	Teacher->NotifyMovementState(NewState);
}
